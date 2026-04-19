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

    // ── Audio Engine ──────────────────────────────────────
    InitAudioDevice();
    Sound sndClick = LoadSound("assets/click.wav");
    Sound sndHonk  = LoadSound("assets/honk.wav");
    Sound sndSiren = LoadSound("assets/siren.wav");
    
    SetSoundVolume(sndClick, 0.6f);
    SetSoundVolume(sndHonk, 0.4f);
    SetSoundVolume(sndSiren, 0.4f);

    // ── Main loop ─────────────────────────────────────────
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Input
        bool uiAction = false;
        if (IsKeyPressed(KEY_S)) { sim.toggleMode(); uiAction = true; }
        if (IsKeyPressed(KEY_R)) { sim.reset(); uiAction = true; }
        if (IsKeyPressed(KEY_Q)) break;
        if (IsKeyPressed(KEY_A)) { sim.spawnAmbulanceManual(); uiAction = true; }
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
            sim.setSpeed(sim.simSpeed() + 0.5f);
            uiAction = true;
        }
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
            sim.setSpeed(sim.simSpeed() - 0.5f);
            uiAction = true;
        }
        if (uiAction) PlaySound(sndClick);

        // Update
        sim.update(dt);

        // ── Audio logic ──────────────────────────────────────
        if (sim.hasActiveAmbulance()) {
            if (!IsSoundPlaying(sndSiren)) PlaySound(sndSiren);
        } else {
            if (IsSoundPlaying(sndSiren)) StopSound(sndSiren);
        }
        
        // Randomly honk if there's massive traffic
        if ((sim.nsQueue() + sim.ewQueue()) > 10) {
            // 60 frames/sec. ~ 1 in 100 frames = ~0.6 honks/sec in bad traffic
            if (GetRandomValue(1, 100) == 1) PlaySound(sndHonk);
        }

        // Draw
        renderer.draw(sim);
    }

    UnloadSound(sndClick);
    UnloadSound(sndHonk);
    UnloadSound(sndSiren);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
