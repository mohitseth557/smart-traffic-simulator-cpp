#pragma once
#include "types.h"
#include <vector>
#include <cmath>

// ─────────────────────────────────────────────────────
//  Vehicle
//  Represents one car approaching the intersection.
//  Handles its own movement, stop-line detection,
//  and gap-keeping to the car ahead.
// ─────────────────────────────────────────────────────
struct Car {
    int   id;
    Dir   dir;
    float x, y;
    float speed;   // px / sec
    float wait;    // seconds spent waiting
    bool  gone;
    Color color;

    Car(int id_, Dir d);

    // Y (NS) or X (EW) coordinate of this car's stop line
    float stopPos() const;

    // True when the car has reached its stop line
    bool atStop() const;

    // True when the car has fully exited the canvas
    bool isPast() const;

    // Distance to the nearest car ahead in the same lane
    float aheadGap(const std::vector<Car>& all) const;
};
