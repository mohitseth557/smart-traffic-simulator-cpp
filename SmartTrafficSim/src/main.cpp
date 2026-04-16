#include "../include/simulation.h"
#include "../include/renderer.h"
#include <algorithm>

int main() {
    // ── Window setup ──────────────────────────────────────
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(WIN_W, WIN_H,
               "Smart Traffic Simulator — Four-Way Intersection");
    SetTargetFPS(60);

    // ── Create sim + renderer ─────────────────────────────
    Simulation sim(true);   // true = Smart AI mode
    Renderer   renderer;

    // ── Main loop ─────────────────────────────────────────
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Input
        if (IsKeyPressed(KEY_S))     sim.toggleMode();
        if (IsKeyPressed(KEY_R))     sim.reset();
        if (IsKeyPressed(KEY_Q))     break;
        if (IsKeyPressed(KEY_EQUAL) ||
            IsKeyPressed(KEY_KP_ADD))
            sim.setSpeed(sim.simSpeed() + 0.5f);
        if (IsKeyPressed(KEY_MINUS) ||
            IsKeyPressed(KEY_KP_SUBTRACT))
            sim.setSpeed(sim.simSpeed() - 0.5f);

        // Update
        sim.update(dt);

        // Draw
        renderer.draw(sim);
    }

    CloseWindow();
    return 0;
}
