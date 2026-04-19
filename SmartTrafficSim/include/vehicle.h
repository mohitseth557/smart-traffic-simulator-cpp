#pragma once
#include "types.h"
#include <vector>
#include <cmath>

// ─────────────────────────────────────────────────────
//  Vehicle
//  Represents one car (or ambulance) approaching the
//  intersection.  Handles movement, stop-line detection,
//  gap-keeping, and turning arcs.
// ─────────────────────────────────────────────────────
struct Car {
    int     id;
    Dir     dir;          // current travel direction
    Dir     entryDir;     // original approach direction
    TurnDir turn;         // STRAIGHT / TURN_LEFT / TURN_RIGHT
    float   x, y;
    float   speed;        // px / sec
    float   wait;         // seconds spent waiting
    bool    queued;
    bool    gone;
    Color   color;
    bool    isAmbulance;

    // ── Turning state ────────────────────────────────
    bool    turning;      // true while executing turn arc
    float   turnProgress; // 0 → 1 through the arc
    float   turnStartX, turnStartY;

    Car(int id_, Dir d, TurnDir t = STRAIGHT, bool ambulance = false);

    // Direction this car will exit the intersection
    Dir  exitDir() const;

    // Y (NS) or X (EW) coordinate of this car's stop line
    float stopPos() const;

    // True when the car has reached its stop line
    bool atStop() const;

    // True when the car has fully exited the canvas
    bool isPast() const;

    // Distance to the nearest car ahead in the same lane
    float aheadGap(const std::vector<Car>& all) const;

    // Current heading angle in degrees (for rendering rotation)
    float headingDeg() const;
};
