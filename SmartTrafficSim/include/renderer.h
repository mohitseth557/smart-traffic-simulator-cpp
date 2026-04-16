#pragma once
#include "simulation.h"
#include "raylib.h"

// ─────────────────────────────────────────────────────
//  Renderer
//  Draws the simulation canvas and the stats panel.
//  All draw calls happen inside Begin/EndDrawing().
// ─────────────────────────────────────────────────────
class Renderer {
public:
    Renderer();
    ~Renderer();

    // Call once per frame inside BeginDrawing / EndDrawing
    void draw(const Simulation& sim);

private:
    // ── Sim canvas helpers ────────────────────────────
    void drawRoad();
    void drawLights(const Simulation& sim);
    void drawCars(const Simulation& sim);
    void drawQueueBars(const Simulation& sim);
    void drawPhaseRing(const Simulation& sim);

    // ── Panel helpers ─────────────────────────────────
    void drawPanel(const Simulation& sim);
    void drawStatCard(float x, float y, float w, float h,
                      const char* label, const char* value,
                      Color valCol = {232,234,240,255});
    void drawQueueBar(float x, float y, float w,
                      const char* label, int q, Color col);

    // ── Primitive helpers ─────────────────────────────
    static void drawLight(float x, float y, Color sig, bool sensor);
    static void dashedHLine(float x1, float x2, float y, Color c);
    static void dashedVLine(float x, float y1, float y2, Color c);

    RenderTexture2D rt_;  // off-screen canvas for the sim view
};
