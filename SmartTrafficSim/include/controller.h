#pragma once
#include "types.h"
#include <string>

// ─────────────────────────────────────────────────────
//  SignalController
//  Manages the 4-phase traffic light cycle.
//
//  Smart mode:  green duration adapts to queue lengths.
//  Fixed mode:  constant BASE_GREEN per phase.
//  Override:    ambulance forces a specific phase green.
// ─────────────────────────────────────────────────────
class SignalController {
public:
    explicit SignalController(bool smart);

    // Advance by dt seconds given current queue lengths
    void  update(float dt, int ns_queue, int ew_queue);

    // Signal colour for a given approach direction
    Color signalColor(Dir d) const;

    // True if currently in green (not yellow/red) for NS or EW
    bool  canGo(Dir d) const;

    // ── Ambulance override ───────────────────────────
    void  requestOverride(Dir ambulanceDir);
    void  clearOverride();
    bool  isOverride() const { return override_; }

    // Accessors
    const std::string& phase()     const { return phase_; }
    bool               isYellow()  const { return yellow_; }
    float              timer()     const { return timer_; }
    float              greenDur()  const { return green_dur_; }
    bool               isSmart()   const { return smart_; }
    void               setSmart(bool v)  { smart_ = v; }

    void reset();

private:
    void advance(float next_green);

    bool        smart_;
    std::string phase_     = "NS";
    bool        yellow_    = false;
    float       timer_     = 0.0f;
    float       green_dur_ = BASE_GREEN;

    // Override state
    bool        override_      = false;
    std::string overridePhase_ = "";
};
