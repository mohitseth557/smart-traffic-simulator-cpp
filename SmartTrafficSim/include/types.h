#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────
//  Simulation constants
// ─────────────────────────────────────────────────────
constexpr int   WIN_W          = 860;
constexpr int   WIN_H          = 700;
constexpr int   SIM_W          = 560;
constexpr int   SIM_H          = 480;
constexpr int   PANEL_X        = SIM_W + 20;
constexpr int   PANEL_W        = WIN_W - PANEL_X - 16;

constexpr float CX             = SIM_W / 2.0f;
constexpr float CY             = SIM_H / 2.0f;
constexpr float ROAD_W         = 54.0f;
constexpr float HALF_ROAD      = ROAD_W / 2.0f;

constexpr float BASE_GREEN     = 20.0f;
constexpr float MIN_GREEN      = 8.0f;
constexpr float MAX_GREEN      = 50.0f;
constexpr float YELLOW_TIME    = 3.0f;
constexpr float SPAWN_INTERVAL = 1.8f;
constexpr int   MAX_PER_DIR    = 12;

// ── Ambulance constants ──────────────────────────────
constexpr float AMBULANCE_SPAWN_INTERVAL = 15.0f;
constexpr float AMBULANCE_SPEED          = 130.0f;

// ── Turn arc radius (matches half-road for smooth arcs)
constexpr float TURN_RADIUS    = HALF_ROAD;

// ─────────────────────────────────────────────────────
//  Colour palette (matches HTML canvas version)
// ─────────────────────────────────────────────────────
inline const Color COL_BG        = {13,  15,  20,  255};
inline const Color COL_GRASS     = {23,  52,   4,  255};
inline const Color COL_ROAD      = {44,  44,  42,  255};
inline const Color COL_LANE      = {68,  68,  65,  255};
inline const Color COL_IBOX      = {186,117,  23,   40};
inline const Color COL_STOPLINE  = {85,  85,  85,  255};
inline const Color COL_GREEN_SIG = {29, 158, 117,  255};
inline const Color COL_YELLOW_SIG= {239,159,  39,  255};
inline const Color COL_RED_SIG   = {226, 75,  74,  255};
inline const Color COL_SENSOR    = {55, 138, 221,  255};
inline const Color COL_POLE      = {136,135, 128,  255};
inline const Color COL_PANEL     = {20,  23,  32,  255};
inline const Color COL_CARD      = {28,  32,  48,  255};
inline const Color COL_TEXT      = {232,234, 240,  255};
inline const Color COL_MUTED     = {107,114, 128,  255};

// ── Ambulance colours ────────────────────────────────
inline const Color COL_AMBULANCE      = {240, 240, 245, 255};
inline const Color COL_AMBULANCE_CROSS= {220,  40,  40, 255};
inline const Color COL_SIREN_RED      = {255,  30,  30, 255};
inline const Color COL_SIREN_BLUE     = { 30,  80, 255, 255};

inline const Color CAR_COLORS[] = {
    {55, 138,221,255},
    {216, 90, 48,255},
    {29, 158,117,255},
    {212, 83,126,255},
    {186,117, 23,255},
    {83,  74,183,255},
    {226, 75, 74,255},
    {15, 110, 86,255},
};
constexpr int NUM_CAR_COLORS = 8;

// ─────────────────────────────────────────────────────
//  Direction enum
// ─────────────────────────────────────────────────────
enum Dir { NORTH=0, SOUTH=1, EAST=2, WEST=3 };

// ─────────────────────────────────────────────────────
//  Turn direction enum
// ─────────────────────────────────────────────────────
enum TurnDir { STRAIGHT=0, TURN_LEFT=1, TURN_RIGHT=2 };

// ─────────────────────────────────────────────────────
//  Per-step stats snapshot
// ─────────────────────────────────────────────────────
struct Snapshot {
    int   step;
    int   ns_queue;
    int   ew_queue;
    int   passed;
    float avg_wait;
    bool  rush;
};
