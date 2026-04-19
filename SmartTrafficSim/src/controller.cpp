#include "../include/controller.h"
#include <algorithm>

SignalController::SignalController(bool smart) : smart_(smart) {}

void SignalController::reset() {
    phase_         = "NS";
    yellow_        = false;
    timer_         = 0.0f;
    green_dur_     = BASE_GREEN;
    override_      = false;
    overridePhase_ = "";
}

// ─────────────────────────────────────────────────────
//  Ambulance override — force phase green instantly
// ─────────────────────────────────────────────────────
void SignalController::requestOverride(Dir ambulanceDir) {
    bool ns = (ambulanceDir == NORTH || ambulanceDir == SOUTH);
    overridePhase_ = ns ? "NS" : "EW";
    phase_         = overridePhase_;
    yellow_        = false;
    timer_         = 0.0f;
    green_dur_     = MAX_GREEN;  // long green while ambulance passes
    override_      = true;
}

void SignalController::clearOverride() {
    override_      = false;
    overridePhase_ = "";
    timer_         = 0.0f;
    green_dur_     = BASE_GREEN;
    // Normal cycling resumes from the current phase
}

// ─────────────────────────────────────────────────────
//  Update — skip phase transitions during override
// ─────────────────────────────────────────────────────
void SignalController::update(float dt, int ns_q, int ew_q) {
    if (override_) {
        // Keep the overridden phase green — don't advance timer
        timer_ = 0.0f;
        return;
    }

    timer_ += dt;
    if (timer_ < green_dur_) return;

    // Compute adaptive green for next phase
    float next = BASE_GREEN;
    if (smart_) {
        if (phase_=="NS")
            next = std::clamp(BASE_GREEN + ns_q*2.0f - (float)ew_q, MIN_GREEN, MAX_GREEN);
        else
            next = std::clamp(BASE_GREEN + ew_q*2.0f - (float)ns_q, MIN_GREEN, MAX_GREEN);
    }
    advance(next);
}

void SignalController::advance(float next_green) {
    if (!yellow_) {
        yellow_    = true;
        timer_     = 0.0f;
        green_dur_ = YELLOW_TIME;
    } else {
        yellow_    = false;
        timer_     = 0.0f;
        phase_     = (phase_=="NS") ? "EW" : "NS";
        green_dur_ = next_green;
    }
}

Color SignalController::signalColor(Dir d) const {
    bool ns = (d==NORTH || d==SOUTH);
    bool serving = (ns && phase_=="NS") || (!ns && phase_=="EW");
    if (serving && !yellow_) return COL_GREEN_SIG;
    if (serving &&  yellow_) return COL_YELLOW_SIG;
    return COL_RED_SIG;
}

bool SignalController::canGo(Dir d) const {
    bool ns = (d==NORTH || d==SOUTH);
    return ((ns ? phase_=="NS" : phase_=="EW") && !yellow_);
}
