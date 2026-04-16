#include "../include/simulation.h"
#include <algorithm>

Simulation::Simulation(bool smart_mode)
    : ctrl_(smart_mode) {}

void Simulation::reset() {
    cars_.clear();
    ctrl_.reset();
    spawnTimer_ = 0;
    nextId_     = 0;
    passed_     = 0;
    spawned_    = 0;
    waitSum_    = 0;
    waitCount_  = 0;
}

void Simulation::toggleMode() {
    ctrl_.setSmart(!ctrl_.isSmart());
    reset();
}

int Simulation::nsQueue() const {
    int n = 0;
    for (auto& c : cars_)
        if (!c.gone && (c.dir==NORTH||c.dir==SOUTH) && c.atStop()) n++;
    return n;
}

int Simulation::ewQueue() const {
    int n = 0;
    for (auto& c : cars_)
        if (!c.gone && (c.dir==EAST||c.dir==WEST) && c.atStop()) n++;
    return n;
}

float Simulation::avgWait() const {
    return waitCount_ > 0 ? waitSum_ / waitCount_ : 0.0f;
}

int Simulation::countDir(Dir d) const {
    int n = 0;
    for (auto& c : cars_) if (!c.gone && c.dir==d) n++;
    return n;
}

void Simulation::spawnCar(float /*dt*/) {
    const Dir dirs[] = {NORTH, SOUTH, EAST, WEST};
    Dir d = dirs[rng_() % 4];
    if (countDir(d) < MAX_PER_DIR) {
        cars_.emplace_back(nextId_++, d);
        spawned_++;
    }
}

void Simulation::moveCars(float dt) {
    float scale = speed_;
    for (auto& c : cars_) {
        if (c.gone) continue;

        bool blocked = !ctrl_.canGo(c.dir) && c.atStop();
        if (blocked) {
            c.wait += dt;
        } else {
            float gap = c.aheadGap(cars_);
            float mv  = (gap > 26.f) ? c.speed * scale * dt : 0.f;
            switch (c.dir) {
                case NORTH: c.y += mv; break;
                case SOUTH: c.y -= mv; break;
                case EAST:  c.x += mv; break;
                case WEST:  c.x -= mv; break;
            }
        }

        if (c.isPast() && !c.gone) {
            c.gone = true;
            passed_++;
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

    moveCars(dt);
}
