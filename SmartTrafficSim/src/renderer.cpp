#include "../include/renderer.h"
#include <algorithm>
#include <cmath>
#include <string>

// ─────────────────────────────────────────────────────
//  Constructor / Destructor
// ─────────────────────────────────────────────────────
Renderer::Renderer() {
    rt_ = LoadRenderTexture(SIM_W, SIM_H);
}
Renderer::~Renderer() {
    UnloadRenderTexture(rt_);
}

// ─────────────────────────────────────────────────────
//  Primitive helpers
// ─────────────────────────────────────────────────────
void Renderer::dashedHLine(float x1, float x2, float y, Color c) {
    for (float x = x1; x < x2; x += 22.f) {
        float end = std::min(x + 12.f, x2);
        DrawLineEx({x, y}, {end, y}, 1.5f, c);
    }
}
void Renderer::dashedVLine(float x, float y1, float y2, Color c) {
    for (float y = y1; y < y2; y += 22.f) {
        float end = std::min(y + 12.f, y2);
        DrawLineEx({x, y}, {x, end}, 1.5f, c);
    }
}

// ─────────────────────────────────────────────────────
//  Traffic light glyph
// ─────────────────────────────────────────────────────
void Renderer::drawLight(float x, float y, Color sig, bool sensor) {
    // Pole
    DrawLineEx({x, y+4}, {x, y+24}, 2.0f, COL_POLE);
    // Housing box
    DrawRectangleRounded({x-7, y-18, 14, 22}, 0.2f, 6, {26,26,26,255});
    DrawRectangleRoundedLinesEx({x-7, y-18, 14, 22}, 0.2f, 6, 0.5f, {60,60,60,255});
    // Bulb
    DrawCircleV({x, y-8}, 5, sig);
    // Halo glow
    Color halo = sig; halo.a = 55;
    DrawCircleV({x, y-8}, 8, halo);
    // Smart sensor dot (blue)
    if (sensor) {
        DrawCircleV({x+9, y-18}, 3, COL_SENSOR);
        Color sdim = COL_SENSOR; sdim.a = 70;
        DrawCircleV({x+9, y-18}, 5, sdim);
    }
}

// ─────────────────────────────────────────────────────
//  Road surface
// ─────────────────────────────────────────────────────
void Renderer::drawRoad() {
    ClearBackground(COL_GRASS);

    // Horizontal road band
    DrawRectangle(0, (int)(CY-HALF_ROAD), SIM_W, (int)ROAD_W, COL_ROAD);
    // Vertical road band
    DrawRectangle((int)(CX-HALF_ROAD), 0, (int)ROAD_W, SIM_H, COL_ROAD);
    // Intersection amber tint
    DrawRectangle((int)(CX-HALF_ROAD), (int)(CY-HALF_ROAD),
                  (int)ROAD_W, (int)ROAD_W, COL_IBOX);

    // Dashed centre lines
    dashedHLine(0,        CX-HALF_ROAD, CY, COL_LANE);
    dashedHLine(CX+HALF_ROAD, SIM_W,   CY, COL_LANE);
    dashedVLine(CX, 0,        CY-HALF_ROAD, COL_LANE);
    dashedVLine(CX, CY+HALF_ROAD, SIM_H,   COL_LANE);

    // Stop lines
    const Color SL = COL_STOPLINE;
    DrawLineEx({CX-HALF_ROAD-2, CY-HALF_ROAD-2},
               {CX-HALF_ROAD-2, CY-HALF_ROAD-18}, 2.5f, SL);
    DrawLineEx({CX+HALF_ROAD+2, CY+HALF_ROAD+2},
               {CX+HALF_ROAD+2, CY+HALF_ROAD+18}, 2.5f, SL);
    DrawLineEx({CX-HALF_ROAD-2, CY+HALF_ROAD+2},
               {CX-HALF_ROAD-18, CY+HALF_ROAD+2}, 2.5f, SL);
    DrawLineEx({CX+HALF_ROAD+2, CY-HALF_ROAD-2},
               {CX+HALF_ROAD+18, CY-HALF_ROAD-2}, 2.5f, SL);
}

// ─────────────────────────────────────────────────────
//  Traffic lights (4 corners)
// ─────────────────────────────────────────────────────
void Renderer::drawLights(const Simulation& sim) {
    bool sn = sim.smartMode();
    drawLight(CX-HALF_ROAD-20, CY-HALF_ROAD-4,  sim.ctrl().signalColor(NORTH), sn);
    drawLight(CX+HALF_ROAD+20, CY+HALF_ROAD+4,  sim.ctrl().signalColor(SOUTH), sn);
    drawLight(CX-HALF_ROAD-4,  CY+HALF_ROAD+20, sim.ctrl().signalColor(EAST),  sn);
    drawLight(CX+HALF_ROAD+4,  CY-HALF_ROAD-20, sim.ctrl().signalColor(WEST),  sn);
}

// ─────────────────────────────────────────────────────
//  Vehicles
// ─────────────────────────────────────────────────────
void Renderer::drawCars(const Simulation& sim) {
    for (const auto& c : sim.cars()) {
        if (c.gone) continue;
        bool vert = (c.dir==NORTH || c.dir==SOUTH);
        float cw = vert ? 14.f : 22.f;
        float ch = vert ? 22.f : 14.f;
        DrawRectangleRounded({c.x-cw/2, c.y-ch/2, cw, ch}, 0.3f, 6, c.color);
        // Windshield tint
        Color wc = {255,255,255,35};
        if (vert) DrawRectangleRounded({c.x-4, c.y-ch/2+3, 8, 5}, 0.3f, 4, wc);
        else      DrawRectangleRounded({c.x-cw/2+3, c.y-4, 5, 8}, 0.3f, 4, wc);
    }
}

// ─────────────────────────────────────────────────────
//  Queue overlay bars (top corners)
// ─────────────────────────────────────────────────────
void Renderer::drawQueueBars(const Simulation& sim) {
    int nsQ = sim.nsQueue();
    int ewQ = sim.ewQueue();

    // NS — top left
    DrawRectangleRounded({8, 8, 88, 36}, 0.25f, 6, {20,23,32,215});
    Color nsC = sim.ctrl().signalColor(NORTH);
    DrawRectangle(12, 13, (int)std::min(nsQ*8.f, 76.f), 10, nsC);
    DrawText(TextFormat("NS: %d", nsQ), 12, 27, 10, COL_MUTED);

    // EW — top right
    DrawRectangleRounded({SIM_W-96.f, 8, 88, 36}, 0.25f, 6, {20,23,32,215});
    Color ewC = sim.ctrl().signalColor(EAST);
    DrawRectangle(SIM_W-92, 13, (int)std::min(ewQ*8.f, 76.f), 10, ewC);
    DrawText(TextFormat("EW: %d", ewQ), SIM_W-92, 27, 10, COL_MUTED);
}

// ─────────────────────────────────────────────────────
//  Phase timer ring at intersection centre
// ─────────────────────────────────────────────────────
void Renderer::drawPhaseRing(const Simulation& sim) {
    float t = sim.ctrl().timer() /
              std::max(sim.ctrl().greenDur(), 0.001f);
    DrawRing({CX,CY}, 9, 13, 0, 360, 32, {80,80,80,70});
    Color rc = (sim.ctrl().phase()=="NS") ? COL_GREEN_SIG : COL_SENSOR;
    DrawRing({CX,CY}, 9, 13, -90, -90+t*360.f, 32, rc);
}

// ─────────────────────────────────────────────────────
//  Stat card helper
// ─────────────────────────────────────────────────────
void Renderer::drawStatCard(float x, float y, float w, float h,
                             const char* label, const char* value,
                             Color valCol) {
    DrawRectangleRounded({x, y, w, h}, 0.2f, 6, COL_CARD);
    DrawText(label, (int)(x+10), (int)(y+8),  10, COL_MUTED);
    DrawText(value, (int)(x+10), (int)(y+24), 18, valCol);
}

// ─────────────────────────────────────────────────────
//  Queue bar helper (panel)
// ─────────────────────────────────────────────────────
void Renderer::drawQueueBar(float x, float y, float w,
                             const char* label, int q, Color col) {
    DrawText(label, (int)x, (int)y, 11, COL_TEXT);
    float tx = x + 38;
    float tw = w - 38;
    DrawRectangleRounded({tx, y, tw, 10}, 0.4f, 4, {40,44,60,255});
    float fw = std::min((float)q / 12.f, 1.0f) * tw;
    if (fw > 0) DrawRectangleRounded({tx, y, fw, 10}, 0.4f, 4, col);
    DrawText(TextFormat("%d", q), (int)(tx+tw+4), (int)y, 10, COL_MUTED);
}

// ─────────────────────────────────────────────────────
//  Side panel
// ─────────────────────────────────────────────────────
void Renderer::drawPanel(const Simulation& sim) {
    float px = (float)PANEL_X;
    float py = 16.0f;
    float pw = (float)PANEL_W;

    // Panel background
    DrawRectangleRounded({px-6, py-6, pw+12, WIN_H-py-4.f},
                         0.08f, 6, COL_PANEL);

    // Title
    DrawText("Smart Traffic", (int)px, (int)py,    16, COL_TEXT);
    DrawText("Simulator",     (int)px, (int)(py+20),16, COL_TEXT);
    py += 50;

    // Mode badge
    const char* ml = sim.smartMode() ? "SMART  AI" : "FIXED TIMER";
    Color mbg  = sim.smartMode() ? Color{20,42,74,255}   : Color{42,32,10,255};
    Color mcol = sim.smartMode() ? Color{122,184,245,255}: Color{245,201,122,255};
    DrawRectangleRounded({px, py, pw, 26}, 0.4f, 6, mbg);
    int tw = MeasureText(ml, 12);
    DrawText(ml, (int)(px+pw/2-tw/2), (int)(py+7), 12, mcol);
    py += 36;

    // Phase name
    const char* pn;
    Color pc;
    if      (sim.ctrl().phase()=="NS" && !sim.ctrl().isYellow()) { pn="NS  GREEN";  pc=COL_GREEN_SIG;  }
    else if (sim.ctrl().phase()=="NS" &&  sim.ctrl().isYellow()) { pn="NS  YELLOW"; pc=COL_YELLOW_SIG; }
    else if (sim.ctrl().phase()=="EW" && !sim.ctrl().isYellow()) { pn="EW  GREEN";  pc=COL_GREEN_SIG;  }
    else                                                           { pn="EW  YELLOW"; pc=COL_YELLOW_SIG; }
    DrawText("Phase", (int)px, (int)py, 10, COL_MUTED);
    DrawText(pn, (int)px, (int)(py+14), 14, pc);
    py += 38;

    // Phase timer progress bar
    float filled = (sim.ctrl().greenDur() > 0)
                   ? (sim.ctrl().timer() / sim.ctrl().greenDur()) * pw : 0;
    DrawRectangleRounded({px, py, pw, 8}, 0.5f, 4, {40,44,60,255});
    Color tbc = (sim.ctrl().phase()=="NS") ? COL_GREEN_SIG : COL_SENSOR;
    if (filled > 0) DrawRectangleRounded({px, py, filled, 8}, 0.5f, 4, tbc);
    py += 16;
    DrawText(TextFormat("%.1fs / %.0fs",
             sim.ctrl().timer(), sim.ctrl().greenDur()),
             (int)px, (int)py, 10, COL_MUTED);
    py += 24;

    // Stat cards
    float cw2 = (pw - 8) / 2.f;
    int aw = (int)sim.avgWait();
    drawStatCard(px, py, pw, 50, "Vehicles cleared",
                 TextFormat("%d", sim.totalPassed()));
    py += 58;
    drawStatCard(px,       py, cw2, 50, "Waiting",
                 TextFormat("%d", sim.nsQueue()+sim.ewQueue()));
    drawStatCard(px+cw2+8, py, cw2, 50, "Avg wait",
                 TextFormat("%ds", aw));
    py += 58;
    drawStatCard(px,       py, cw2, 50, "Spawned",
                 TextFormat("%d", sim.totalSpawned()));
    drawStatCard(px+cw2+8, py, cw2, 50, "Speed",
                 TextFormat("%.1fx", sim.simSpeed()));
    py += 64;

    // Queue bars
    DrawText("Queues", (int)px, (int)py, 10, COL_MUTED);
    py += 14;
    drawQueueBar(px, py,    pw, "N/S",
                 sim.nsQueue(), sim.ctrl().signalColor(NORTH));
    drawQueueBar(px, py+18, pw, "E/W",
                 sim.ewQueue(), sim.ctrl().signalColor(EAST));
    py += 48;

    // Key hints
    DrawText("[S] Smart / Fixed",    (int)px, (int)py,     10, COL_MUTED);
    DrawText("[R] Reset",            (int)px, (int)(py+14),10, COL_MUTED);
    DrawText("[+]/[-] Speed",        (int)px, (int)(py+28),10, COL_MUTED);
    DrawText("[Q] Quit",             (int)px, (int)(py+42),10, COL_MUTED);
}

// ─────────────────────────────────────────────────────
//  Master draw — called once per frame
// ─────────────────────────────────────────────────────
void Renderer::draw(const Simulation& sim) {
    // Draw sim scene into off-screen texture
    BeginTextureMode(rt_);
        drawRoad();
        drawLights(sim);
        drawCars(sim);
        drawQueueBars(sim);
        drawPhaseRing(sim);
    EndTextureMode();

    BeginDrawing();
        ClearBackground(COL_BG);

        // Blit sim texture (flip Y — raylib RT is upside down)
        DrawTexturePro(
            rt_.texture,
            {0, 0, (float)SIM_W, -(float)SIM_H},
            {8, 8, (float)SIM_W, (float)SIM_H},
            {0, 0}, 0, WHITE
        );
        // Canvas border
        DrawRectangleRoundedLinesEx({8,8,(float)SIM_W,(float)SIM_H},
                                    0.04f, 6, 1.0f, {42,47,66,255});

        // Side panel
        drawPanel(sim);

        // Bottom bar
        DrawRectangle(0, WIN_H-32, WIN_W, 32, {10,12,18,255});
        DrawText("Smart Traffic Simulator  |  C++17 + raylib",
                 12, WIN_H-20, 10, COL_MUTED);
        DrawText(TextFormat("FPS: %d", GetFPS()),
                 WIN_W-65, WIN_H-20, 10, COL_MUTED);

    EndDrawing();
}
