#include "../include/vehicle.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────
Car::Car(int id_, Dir d, TurnDir t, bool ambulance)
    : id(id_), dir(d), entryDir(d), turn(t),
      wait(0), queued(false), gone(false), isAmbulance(ambulance),
      turning(false), turnProgress(0),
      turnStartX(0), turnStartY(0)
{
    if (ambulance) {
        color = COL_AMBULANCE;
        speed = AMBULANCE_SPEED;
    } else {
        color = CAR_COLORS[id_ % NUM_CAR_COLORS];
        speed = 80.0f + (float)(id_ % 5) * 8.0f;
    }

    // All cars use the same lane — no offset for right turns
    switch (d) {
        case NORTH: x = CX - 13; y = -20;        break;
        case SOUTH: x = CX + 13; y = SIM_H + 20; break;
        case EAST:  x = -20;     y = CY + 13;    break;
        case WEST:  x = SIM_W+20;y = CY - 13;    break;
    }
}

// ─────────────────────────────────────────────────────
//  Exit direction based on entry + turn
//
//  NOTE: Dir naming is by approach, not heading:
//    NORTH → moves south (y+)    SOUTH → moves north (y-)
//    EAST  → moves east  (x+)    WEST  → moves west  (x-)
//
//  Left/right from driver's physical perspective:
//    NORTH(heading south): left=east,  right=west
//    SOUTH(heading north): left=west,  right=east
//    EAST (heading east):  left=north→ use Dir SOUTH (moves y-)
//    WEST (heading west):  left=south→ use Dir NORTH (moves y+)
// ─────────────────────────────────────────────────────
Dir Car::exitDir() const {
    if (turn == STRAIGHT) return entryDir;
    if (turn == TURN_LEFT) {
        switch (entryDir) {
            case NORTH: return EAST;
            case SOUTH: return WEST;
            case EAST:  return SOUTH;  // physically turns north (y-), Dir SOUTH does y-
            case WEST:  return NORTH;  // physically turns south (y+), Dir NORTH does y+
        }
    } else { // TURN_RIGHT
        switch (entryDir) {
            case NORTH: return WEST;
            case SOUTH: return EAST;
            case EAST:  return NORTH;  // physically turns south (y+), Dir NORTH does y+
            case WEST:  return SOUTH;  // physically turns north (y-), Dir SOUTH does y-
        }
    }
    return entryDir;
}

// ─────────────────────────────────────────────────────
//  Bezier control points for the turn arc
// ─────────────────────────────────────────────────────
static void getBezierPoints(Dir entry, TurnDir turn,
                            float& p0x, float& p0y,
                            float& p1x, float& p1y,
                            float& p2x, float& p2y)
{
    // Entry points (at intersection edge)
    // Exit points = lane position on exit road at intersection edge
    // P1 = tangent intersection (creates smooth L-shaped curve)

    if (entry == NORTH && turn == TURN_LEFT) {
        // heading south, turn east. Exit = EAST lane
        p0x=CX-13; p0y=CY-HALF_ROAD;  p1x=CX-13; p1y=CY+13;  p2x=CX+HALF_ROAD; p2y=CY+13;
    } else if (entry == NORTH && turn == TURN_RIGHT) {
        // heading south, turn west. Exit = WEST lane
        p0x=CX-13; p0y=CY-HALF_ROAD;  p1x=CX-13; p1y=CY-13;  p2x=CX-HALF_ROAD; p2y=CY-13;
    } else if (entry == SOUTH && turn == TURN_LEFT) {
        // heading north, turn west
        p0x=CX+13; p0y=CY+HALF_ROAD;  p1x=CX+13; p1y=CY-13;  p2x=CX-HALF_ROAD; p2y=CY-13;
    } else if (entry == SOUTH && turn == TURN_RIGHT) {
        // heading north, turn east
        p0x=CX+13; p0y=CY+HALF_ROAD;  p1x=CX+13; p1y=CY+13;  p2x=CX+HALF_ROAD; p2y=CY+13;
    } else if (entry == EAST && turn == TURN_LEFT) {
        // heading east, turn north (physically). Exit as Dir SOUTH (y-), lane x=CX+13
        p0x=CX-HALF_ROAD; p0y=CY+13;  p1x=CX+13; p1y=CY+13;  p2x=CX+13; p2y=CY-HALF_ROAD;
    } else if (entry == EAST && turn == TURN_RIGHT) {
        // heading east, turn south (physically). Exit as Dir NORTH (y+), lane x=CX-13
        p0x=CX-HALF_ROAD; p0y=CY+13;  p1x=CX-13; p1y=CY+13;  p2x=CX-13; p2y=CY+HALF_ROAD;
    } else if (entry == WEST && turn == TURN_LEFT) {
        // heading west, turn south (physically). Exit as Dir NORTH (y+), lane x=CX-13
        p0x=CX+HALF_ROAD; p0y=CY-13;  p1x=CX-13; p1y=CY-13;  p2x=CX-13; p2y=CY+HALF_ROAD;
    } else { // WEST + RIGHT
        // heading west, turn north (physically). Exit as Dir SOUTH (y-), lane x=CX+13
        p0x=CX+HALF_ROAD; p0y=CY-13;  p1x=CX+13; p1y=CY-13;  p2x=CX+13; p2y=CY-HALF_ROAD;
    }
}

// ─────────────────────────────────────────────────────
//  Stop line position
// ─────────────────────────────────────────────────────
float Car::stopPos() const {
    switch (entryDir) {
        case NORTH: return CY - HALF_ROAD - 16;
        case SOUTH: return CY + HALF_ROAD + 16;
        case EAST:  return CX - HALF_ROAD - 16;
        case WEST:  return CX + HALF_ROAD + 16;
    }
    return 0;
}

bool Car::atStop() const {
    if (turning) return false;
    switch (entryDir) {
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

// ─────────────────────────────────────────────────────
//  Gap to nearest car ahead in the same approach lane
// ─────────────────────────────────────────────────────
float Car::aheadGap(const std::vector<Car>& all) const {
    float minD = 9999.f;
    float rad = headingDeg() * (float)M_PI / 180.0f;
    float hx = cosf(rad);
    float hy = sinf(rad);

    for (const auto& o : all) {
        if (&o == this || o.gone) continue;
        
        float dx = o.x - x;
        float dy = o.y - y;
        
        float fwd = dx * hx + dy * hy;
        float lat = std::fabs(dx * (-hy) + dy * hx);
        
        // A standard lane is 27px wide. If absolute lateral distance < 16, they are in our path.
        if (fwd > 0.1f && lat < 16.0f) {
            if (fwd < minD) minD = fwd;
        }
    }
    return minD;
}

// ─────────────────────────────────────────────────────
//  Heading angle in degrees from Bezier tangent
// ─────────────────────────────────────────────────────
float Car::headingDeg() const {
    if (!turning) {
        // Straight heading based on physical movement direction
        switch (dir) {
            case NORTH: return  90.0f;  // moves south (y+) = 90° clockwise
            case SOUTH: return -90.0f;  // moves north (y-) = -90°
            case EAST:  return   0.0f;  // moves east  (x+) = 0°
            case WEST:  return 180.0f;  // moves west  (x-) = 180°
        }
    }

    // Compute tangent from Bezier derivative: B'(t) = 2(1-t)(P1-P0) + 2t(P2-P1)
    float p0x,p0y, p1x,p1y, p2x,p2y;
    getBezierPoints(entryDir, turn, p0x,p0y, p1x,p1y, p2x,p2y);

    float t = turnProgress;
    float dx = 2*(1-t)*(p1x-p0x) + 2*t*(p2x-p1x);
    float dy = 2*(1-t)*(p1y-p0y) + 2*t*(p2y-p1y);
    return atan2f(dy, dx) * 180.0f / (float)M_PI;
}
