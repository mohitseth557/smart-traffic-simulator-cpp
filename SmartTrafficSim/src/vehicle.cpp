#include "../include/vehicle.h"
#include <algorithm>

Car::Car(int id_, Dir d)
    : id(id_), dir(d), wait(0), gone(false) {
    color = CAR_COLORS[id_ % NUM_CAR_COLORS];
    speed = 80.0f + (float)(id_ % 5) * 8.0f;
    switch (d) {
        case NORTH: x = CX - 13; y = -20;       break;
        case SOUTH: x = CX + 13; y = SIM_H + 20; break;
        case EAST:  x = -20;     y = CY + 13;   break;
        case WEST:  x = SIM_W+20;y = CY - 13;   break;
    }
}

float Car::stopPos() const {
    switch (dir) {
        case NORTH: return CY - HALF_ROAD - 16;
        case SOUTH: return CY + HALF_ROAD + 16;
        case EAST:  return CX - HALF_ROAD - 16;
        case WEST:  return CX + HALF_ROAD + 16;
    }
    return 0;
}

bool Car::atStop() const {
    switch (dir) {
        case NORTH: return y >= stopPos()-2 && y < CY;
        case SOUTH: return y <= stopPos()+2 && y > CY;
        case EAST:  return x >= stopPos()-2 && x < CX;
        case WEST:  return x <= stopPos()+2 && x > CX;
    }
    return false;
}

bool Car::isPast() const {
    return y > SIM_H+40 || y < -40 || x > SIM_W+40 || x < -40;
}

float Car::aheadGap(const std::vector<Car>& all) const {
    float minD = 9999.f;
    for (const auto& o : all) {
        if (&o == this || o.dir != dir || o.gone) continue;
        float d = 9999.f;
        switch (dir) {
            case NORTH: d = o.y - y; break;
            case SOUTH: d = y - o.y; break;
            case EAST:  d = o.x - x; break;
            case WEST:  d = x - o.x; break;
        }
        if (d > 0 && d < minD) minD = d;
    }
    return minD;
}
