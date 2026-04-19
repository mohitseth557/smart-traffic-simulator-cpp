#pragma once
#include "types.h"
#include "vehicle.h"
#include "controller.h"
#include <vector>
#include <random>
#include <deque>

// ─────────────────────────────────────────────────────
//  Simulation
//  Owns all vehicles and the signal controller.
//  Call update(dt) every frame.
// ─────────────────────────────────────────────────────
class Simulation {
public:
    explicit Simulation(bool smart_mode = true);

    void update(float dt);
    void reset();
    void toggleMode();

    // ── Manual ambulance spawn (KEY_A) ───────────────
    void spawnAmbulanceManual();

    // Accessors
    const std::vector<Car>&  cars()       const { return cars_; }
    const SignalController&  ctrl()       const { return ctrl_; }
    float                    simSpeed()   const { return speed_; }
    void                     setSpeed(float s)  { speed_ = std::clamp(s, 0.5f, 5.0f); }
    int                      nsQueue()    const;
    int                      ewQueue()    const;
    int                      totalPassed()const { return passed_; }
    int                      totalSpawned()const{ return spawned_; }
    float                    avgWait()    const;
    bool                     smartMode()  const { return ctrl_.isSmart(); }

    // ── Ambulance accessors ──────────────────────────
    int  ambulancesPassed()   const { return ambulancesPassed_; }
    bool hasActiveAmbulance() const;

    const std::deque<int>& getNsHistory() const { return nsHistory_; }
    const std::deque<int>& getEwHistory() const { return ewHistory_; }

private:
    void spawnCar(float dt);
    void spawnAmbulance(float dt);
    void moveCars(float dt);
    void updateTurns(float dt);
    int  countDir(Dir d) const;

    std::vector<Car>  cars_;
    SignalController  ctrl_;
    std::mt19937      rng_{42};

    float  speed_            = 1.5f;
    float  spawnTimer_       = 0.0f;
    int    nextId_           = 0;
    int    passed_           = 0;
    int    spawned_          = 0;
    float  waitSum_          = 0.0f;
    int    waitCount_        = 0;

    // ── Ambulance state ──────────────────────────────
    float  ambulanceTimer_   = 0.0f;
    int    ambulancesPassed_ = 0;

    // ── Density Graph State ──────────────────────────
    std::deque<int> nsHistory_;
    std::deque<int> ewHistory_;
    float historyTimer_ = 0.0f;
};
