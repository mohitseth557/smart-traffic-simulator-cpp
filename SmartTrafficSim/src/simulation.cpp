#include "../include/simulation.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────
//  Bezier helper (same logic as in vehicle.cpp)
// ─────────────────────────────────────────────────────
static void getBezierPoints(Dir entry, TurnDir turn,
                            float& p0x, float& p0y,
                            float& p1x, float& p1y,
                            float& p2x, float& p2y)
{
    if (entry == NORTH && turn == TURN_LEFT) {
        p0x=CX-13; p0y=CY-HALF_ROAD;  p1x=CX-13; p1y=CY+13;  p2x=CX+HALF_ROAD; p2y=CY+13;
    } else if (entry == NORTH && turn == TURN_RIGHT) {
        p0x=CX-13; p0y=CY-HALF_ROAD;  p1x=CX-13; p1y=CY-13;  p2x=CX-HALF_ROAD; p2y=CY-13;
    } else if (entry == SOUTH && turn == TURN_LEFT) {
        p0x=CX+13; p0y=CY+HALF_ROAD;  p1x=CX+13; p1y=CY-13;  p2x=CX-HALF_ROAD; p2y=CY-13;
    } else if (entry == SOUTH && turn == TURN_RIGHT) {
        p0x=CX+13; p0y=CY+HALF_ROAD;  p1x=CX+13; p1y=CY+13;  p2x=CX+HALF_ROAD; p2y=CY+13;
    } else if (entry == EAST && turn == TURN_LEFT) {
        p0x=CX-HALF_ROAD; p0y=CY+13;  p1x=CX+13; p1y=CY+13;  p2x=CX+13; p2y=CY-HALF_ROAD;
    } else if (entry == EAST && turn == TURN_RIGHT) {
        p0x=CX-HALF_ROAD; p0y=CY+13;  p1x=CX-13; p1y=CY+13;  p2x=CX-13; p2y=CY+HALF_ROAD;
    } else if (entry == WEST && turn == TURN_LEFT) {
        p0x=CX+HALF_ROAD; p0y=CY-13;  p1x=CX-13; p1y=CY-13;  p2x=CX-13; p2y=CY+HALF_ROAD;
    } else { // WEST + RIGHT
        p0x=CX+HALF_ROAD; p0y=CY-13;  p1x=CX+13; p1y=CY-13;  p2x=CX+13; p2y=CY-HALF_ROAD;
    }
}

// Quadratic Bezier: B(t) = (1-t)^2 * P0 + 2(1-t)t * P1 + t^2 * P2
static float bezier(float t, float p0, float p1, float p2) {
    float u = 1.0f - t;
    return u*u*p0 + 2*u*t*p1 + t*t*p2;
}

// Approximate arc length of Bezier for speed normalization
static float bezierLength(float p0x, float p0y, float p1x, float p1y,
                           float p2x, float p2y) {
    float len = 0;
    float px = p0x, py = p0y;
    for (int i = 1; i <= 20; i++) {
        float t = (float)i / 20.0f;
        float nx = bezier(t, p0x, p1x, p2x);
        float ny = bezier(t, p0y, p1y, p2y);
        float dx = nx - px, dy = ny - py;
        len += sqrtf(dx*dx + dy*dy);
        px = nx; py = ny;
    }
    return len;
}

// ─────────────────────────────────────────────────────

Simulation::Simulation(bool smart_mode)
    : ctrl_(smart_mode) {}

void Simulation::reset() {
    cars_.clear();
    ctrl_.reset();
    spawnTimer_       = 0;
    nextId_           = 0;
    passed_           = 0;
    spawned_          = 0;
    waitSum_          = 0;
    waitCount_        = 0;
    ambulanceTimer_   = 0;
    ambulancesPassed_ = 0;
    nsHistory_.clear();
    ewHistory_.clear();
    historyTimer_     = 0.0f;
}

void Simulation::toggleMode() {
    ctrl_.setSmart(!ctrl_.isSmart());
    reset();
}

int Simulation::nsQueue() const {
    int n = 0;
    for (auto& c : cars_)
        if (!c.gone && (c.entryDir==NORTH||c.entryDir==SOUTH) && c.queued) n++;
    return n;
}

int Simulation::ewQueue() const {
    int n = 0;
    for (auto& c : cars_)
        if (!c.gone && (c.entryDir==EAST||c.entryDir==WEST) && c.queued) n++;
    return n;
}

float Simulation::avgWait() const {
    return waitCount_ > 0 ? waitSum_ / waitCount_ : 0.0f;
}

int Simulation::countDir(Dir d) const {
    int n = 0;
    for (auto& c : cars_) if (!c.gone && c.entryDir==d) n++;
    return n;
}

bool Simulation::hasActiveAmbulance() const {
    for (auto& c : cars_)
        if (!c.gone && c.isAmbulance) return true;
    return false;
}

// ─────────────────────────────────────────────────────
//  Spawn a normal car with random turn direction
// ─────────────────────────────────────────────────────
void Simulation::spawnCar(float /*dt*/) {
    const Dir dirs[] = {NORTH, SOUTH, EAST, WEST};
    Dir d = dirs[rng_() % 4];
    if (countDir(d) < MAX_PER_DIR) {
        int r = rng_() % 10;
        TurnDir td = STRAIGHT;
        if (r < 3)      td = TURN_LEFT;
        else if (r < 6) td = TURN_RIGHT;
        cars_.emplace_back(nextId_++, d, td, false);
        spawned_++;
    }
}

// ─────────────────────────────────────────────────────
//  Spawn an ambulance (auto-timer)
// ─────────────────────────────────────────────────────
void Simulation::spawnAmbulance(float dt) {
    ambulanceTimer_ += dt;
    if (ambulanceTimer_ < AMBULANCE_SPAWN_INTERVAL) return;
    ambulanceTimer_ = 0;
    if (hasActiveAmbulance()) return;

    const Dir dirs[] = {NORTH, SOUTH, EAST, WEST};
    Dir d = dirs[rng_() % 4];
    cars_.emplace_back(nextId_++, d, STRAIGHT, true);
    spawned_++;
    ctrl_.requestOverride(d);
}

void Simulation::spawnAmbulanceManual() {
    if (hasActiveAmbulance()) return;
    const Dir dirs[] = {NORTH, SOUTH, EAST, WEST};
    Dir d = dirs[rng_() % 4];
    cars_.emplace_back(nextId_++, d, STRAIGHT, true);
    spawned_++;
    ctrl_.requestOverride(d);
    ambulanceTimer_ = 0;
}

// ─────────────────────────────────────────────────────
//  Update turning cars — Bezier interpolation
// ─────────────────────────────────────────────────────
void Simulation::updateTurns(float dt) {
    float scale = speed_;
    for (auto& c : cars_) {
        if (c.gone || !c.turning) continue;

        // Get Bezier control points
        float p0x,p0y, p1x,p1y, p2x,p2y;
        getBezierPoints(c.entryDir, c.turn, p0x,p0y, p1x,p1y, p2x,p2y);

        // Advance progress based on speed / arc length
        float arcLen = bezierLength(p0x,p0y, p1x,p1y, p2x,p2y);
        if (arcLen < 1.0f) arcLen = 1.0f;
        float arcSpeed = c.speed * scale;
        c.turnProgress += (arcSpeed * dt) / arcLen;

        if (c.turnProgress >= 1.0f) {
            // Turn complete — snap to exit position
            c.turnProgress = 1.0f;
            c.turning = false;
            c.dir = c.exitDir();
            c.x = p2x;  // Bezier endpoint = correct exit position
            c.y = p2y;
            continue;
        }

        // Interpolate position along Bezier
        c.x = bezier(c.turnProgress, p0x, p1x, p2x);
        c.y = bezier(c.turnProgress, p0y, p1y, p2y);
    }
}

// ─────────────────────────────────────────────────────
//  Move cars (straight + turn initiation)
// ─────────────────────────────────────────────────────
void Simulation::moveCars(float dt) {
    float scale = speed_;

    for (auto& c : cars_) {
        if (c.gone || c.turning) continue;

        bool redLight = !ctrl_.canGo(c.entryDir) && !c.isAmbulance;
        float gap = c.aheadGap(cars_);
        
        if (redLight) {
            float distToStop = 9999.f;
            switch (c.entryDir) {
                case NORTH: distToStop = c.stopPos() - c.y; break;
                case SOUTH: distToStop = c.y - c.stopPos(); break;
                case EAST:  distToStop = c.stopPos() - c.x; break;
                case WEST:  distToStop = c.x - c.stopPos(); break;
            }
            if (distToStop > 0) {
                float effectiveGap = distToStop + 24.f; // stops when gap < 26
                if (effectiveGap < gap) gap = effectiveGap;
            } else if (distToStop <= 0 && distToStop > -20.f) {
                gap = 0.f; // hard stop if at or slightly past line
            }
        }

        float targetSpeed = c.speed;
        if (gap < 26.f) {
            targetSpeed = 0.f;
        } else if (gap < 80.f) {
            targetSpeed = c.speed * ((gap - 26.f) / 54.f);
        }

        c.queued = (targetSpeed < c.speed * 0.1f && !c.isPast() && !c.turning);

        float mv = targetSpeed * scale * dt;
        
        if (mv == 0.f && redLight && gap <= 26.f) {
            c.wait += dt;
        }

            // Check if car should start turning
            bool inBox = (c.x > CX - HALF_ROAD && c.x < CX + HALF_ROAD &&
                          c.y > CY - HALF_ROAD && c.y < CY + HALF_ROAD);

            if (inBox && c.turn != STRAIGHT && !c.turning) {
                c.turning = true;
                c.turnProgress = 0.0f;
                c.turnStartX = c.x;
                c.turnStartY = c.y;
                continue;
            }

            // Straight movement
            switch (c.dir) {
                case NORTH: c.y += mv; break;
                case SOUTH: c.y -= mv; break;
                case EAST:  c.x += mv; break;
                case WEST:  c.x -= mv; break;
            }
    }

    // Handle turns (Bezier interpolation)
    updateTurns(dt);

    // Mark cars that have exited
    for (auto& c : cars_) {
        if (c.gone) continue;
        if (c.isPast()) {
            c.gone = true;
            passed_++;
            if (c.isAmbulance) {
                ambulancesPassed_++;
                ctrl_.clearOverride();
            }
            waitSum_   += c.wait;
            waitCount_++;
        }
    }

    // Purge gone cars
    cars_.erase(
        std::remove_if(cars_.begin(), cars_.end(),
                       [](const Car& c){ return c.gone; }),
        cars_.end());
}

void Simulation::update(float dt) {
    ctrl_.update(dt, nsQueue(), ewQueue());

    spawnTimer_ += dt;
    if (spawnTimer_ >= SPAWN_INTERVAL / speed_) {
        spawnTimer_ = 0;
        spawnCar(dt);
    }

    spawnAmbulance(dt);
    moveCars(dt);

    // ── Density History Update ───────────────────────
    historyTimer_ += dt;
    if (historyTimer_ >= 1.0f / speed_) {
        historyTimer_ = 0.0f;
        int ns_density = countDir(NORTH) + countDir(SOUTH);
        int ew_density = countDir(EAST) + countDir(WEST);
        nsHistory_.push_back(ns_density);
        ewHistory_.push_back(ew_density);
        if (nsHistory_.size() > 60) {
            nsHistory_.pop_front();
            ewHistory_.pop_front();
        }
    }
}
