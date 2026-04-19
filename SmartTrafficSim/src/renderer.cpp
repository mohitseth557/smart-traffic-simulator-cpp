#include "../include/renderer.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <ctime>
#include <deque>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
    DrawLineEx({x, y+4}, {x, y+24}, 2.0f, COL_POLE);
    DrawRectangleRounded({x-7, y-18, 14, 22}, 0.2f, 6, {26,26,26,255});
    DrawRectangleRoundedLinesEx({x-7, y-18, 14, 22}, 0.2f, 6, 0.5f, {60,60,60,255});
    DrawCircleV({x, y-8}, 5, sig);
    Color halo = sig; halo.a = 55;
    DrawCircleV({x, y-8}, 8, halo);
    if (sensor) {
        DrawCircleV({x+9, y-18}, 3, COL_SENSOR);
        Color sdim = COL_SENSOR; sdim.a = 70;
        DrawCircleV({x+9, y-18}, 5, sdim);
    }
}

// ─────────────────────────────────────────────────────
//  Small turn-arrow indicator on road
// ─────────────────────────────────────────────────────
static void drawTurnArrow(float x, float y, float rot, Color c) {
    Vector2 v1 = {0, -4};
    Vector2 v2 = {-3,  3};
    Vector2 v3 = { 3,  3};
    float rad = rot * (float)M_PI / 180.0f;
    float cs = cosf(rad), sn = sinf(rad);
    auto rotPt = [&](Vector2 v) -> Vector2 {
        return {x + v.x*cs - v.y*sn, y + v.x*sn + v.y*cs};
    };
    DrawTriangle(rotPt(v1), rotPt(v3), rotPt(v2), c);
}

// ─────────────────────────────────────────────────────
//  Road surface with arched corners
// ─────────────────────────────────────────────────────
void Renderer::drawRoad() {
    ClearBackground(COL_GRASS);

    // Horizontal road band
    DrawRectangle(0, (int)(CY-HALF_ROAD), SIM_W, (int)ROAD_W, COL_ROAD);
    // Vertical road band
    DrawRectangle((int)(CX-HALF_ROAD), 0, (int)ROAD_W, SIM_H, COL_ROAD);
    // Intersection centre tint
    DrawRectangle((int)(CX-HALF_ROAD), (int)(CY-HALF_ROAD),
                  (int)ROAD_W, (int)ROAD_W, COL_IBOX);

    // ── Arched road corners ──────────────────────────
    // Draw grass-colored quarter-circles at the 4 outer
    // corners where the roads meet to create smooth arcs.
    // Radius kept small (8px) so it doesn't overlap lanes.
    float R = 8.0f;
    Color gc = COL_GRASS;

    // Top-left corner (where top of E-W road meets left of N-S road)
    DrawCircleSector({CX-HALF_ROAD, CY-HALF_ROAD}, R, 0, 90, 16, gc);
    // Top-right corner
    DrawCircleSector({CX+HALF_ROAD, CY-HALF_ROAD}, R, 90, 180, 16, gc);
    // Bottom-right corner
    DrawCircleSector({CX+HALF_ROAD, CY+HALF_ROAD}, R, 180, 270, 16, gc);
    // Bottom-left corner
    DrawCircleSector({CX-HALF_ROAD, CY+HALF_ROAD}, R, 270, 360, 16, gc);

    // ── Road edge arc outlines (subtle curb effect) ──
    Color curbCol = {75, 78, 65, 160};
    DrawCircleSectorLines({CX-HALF_ROAD, CY-HALF_ROAD}, R, 0, 90, 16, curbCol);
    DrawCircleSectorLines({CX+HALF_ROAD, CY-HALF_ROAD}, R, 90, 180, 16, curbCol);
    DrawCircleSectorLines({CX+HALF_ROAD, CY+HALF_ROAD}, R, 180, 270, 16, curbCol);
    DrawCircleSectorLines({CX-HALF_ROAD, CY+HALF_ROAD}, R, 270, 360, 16, curbCol);

    // ── Curb edge lines along road edges ─────────────
    // Vertical road edges
    DrawLineEx({CX-HALF_ROAD, 0}, {CX-HALF_ROAD, CY-HALF_ROAD}, 1.0f, curbCol);
    DrawLineEx({CX-HALF_ROAD, CY+HALF_ROAD}, {CX-HALF_ROAD, (float)SIM_H}, 1.0f, curbCol);
    DrawLineEx({CX+HALF_ROAD, 0}, {CX+HALF_ROAD, CY-HALF_ROAD}, 1.0f, curbCol);
    DrawLineEx({CX+HALF_ROAD, CY+HALF_ROAD}, {CX+HALF_ROAD, (float)SIM_H}, 1.0f, curbCol);
    // Horizontal road edges
    DrawLineEx({0, CY-HALF_ROAD}, {CX-HALF_ROAD, CY-HALF_ROAD}, 1.0f, curbCol);
    DrawLineEx({CX+HALF_ROAD, CY-HALF_ROAD}, {(float)SIM_W, CY-HALF_ROAD}, 1.0f, curbCol);
    DrawLineEx({0, CY+HALF_ROAD}, {CX-HALF_ROAD, CY+HALF_ROAD}, 1.0f, curbCol);
    DrawLineEx({CX+HALF_ROAD, CY+HALF_ROAD}, {(float)SIM_W, CY+HALF_ROAD}, 1.0f, curbCol);

    // Dashed centre lines
    dashedHLine(0,             CX-HALF_ROAD, CY, COL_LANE);
    dashedHLine(CX+HALF_ROAD,  SIM_W,       CY, COL_LANE);
    dashedVLine(CX, 0,             CY-HALF_ROAD, COL_LANE);
    dashedVLine(CX, CY+HALF_ROAD, SIM_H,       COL_LANE);

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

    // Turn arrows on road surface
    Color arrowCol = {80, 80, 75, 100};
    drawTurnArrow(CX - 20, CY - HALF_ROAD - 36, 90, arrowCol);
    drawTurnArrow(CX - 6,  CY - HALF_ROAD - 36, 180, arrowCol);
    drawTurnArrow(CX + 8,  CY - HALF_ROAD - 36, -90, arrowCol);
    drawTurnArrow(CX + 20, CY + HALF_ROAD + 36, -90, arrowCol);
    drawTurnArrow(CX + 6,  CY + HALF_ROAD + 36, 0, arrowCol);
    drawTurnArrow(CX - 8,  CY + HALF_ROAD + 36, 90, arrowCol);
    drawTurnArrow(CX - HALF_ROAD - 36, CY + 20, 0, arrowCol);
    drawTurnArrow(CX - HALF_ROAD - 36, CY + 6, 90, arrowCol);
    drawTurnArrow(CX - HALF_ROAD - 36, CY - 8, 180, arrowCol);
    drawTurnArrow(CX + HALF_ROAD + 36, CY - 20, 180, arrowCol);
    drawTurnArrow(CX + HALF_ROAD + 36, CY - 6, -90, arrowCol);
    drawTurnArrow(CX + HALF_ROAD + 36, CY + 8, 0, arrowCol);
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
//  Vehicles — simple rectangular cars (original style)
// ─────────────────────────────────────────────────────
void Renderer::drawCars(const Simulation& sim) {
    double t = GetTime();

    for (const auto& c : sim.cars()) {
        if (c.gone) continue;

        if (c.isAmbulance) {
            // ── Ambulance ────────────────────────────
            bool vert = (c.dir==NORTH || c.dir==SOUTH);
            // During turn use heading to determine orientation
            if (c.turning) {
                float h = c.headingDeg();
                float absH = fabsf(h);
                vert = (absH > 45.f && absH < 135.f);
            }
            float aw = vert ? 16.f : 26.f;
            float ah = vert ? 26.f : 16.f;

            // White body
            DrawRectangleRounded({c.x-aw/2, c.y-ah/2, aw, ah},
                                 0.35f, 6, COL_AMBULANCE);

            // Red cross
            float cs = 4.f;
            DrawLineEx({c.x-cs, c.y}, {c.x+cs, c.y}, 2.5f, COL_AMBULANCE_CROSS);
            DrawLineEx({c.x, c.y-cs}, {c.x, c.y+cs}, 2.5f, COL_AMBULANCE_CROSS);

            // Flashing siren
            bool flash = ((int)(t * 6.0) % 2) == 0;
            Color sirenCol = flash ? COL_SIREN_RED : COL_SIREN_BLUE;
            float sy = vert ? c.y - ah/2 + 3 : c.y;
            float sx = vert ? c.x : c.x - aw/2 + 3;
            DrawCircleV({sx, sy}, 3, sirenCol);
            Color glow = sirenCol; glow.a = 40;
            DrawCircleV({sx, sy}, 8, glow);
            glow.a = 20;
            DrawCircleV({sx, sy}, 14, glow);

        } else {
            // ── Normal car (original rectangle style) ─
            bool vert = (c.dir==NORTH || c.dir==SOUTH);
            if (c.turning) {
                float h = c.headingDeg();
                float absH = fabsf(h);
                vert = (absH > 45.f && absH < 135.f);
            }
            float cw = vert ? 14.f : 22.f;
            float ch = vert ? 22.f : 14.f;

            DrawRectangleRounded({c.x-cw/2, c.y-ch/2, cw, ch},
                                 0.3f, 6, c.color);
            // Windshield tint
            Color wc = {255,255,255,35};
            if (vert)
                DrawRectangleRounded({c.x-4, c.y-ch/2+3, 8, 5}, 0.3f, 4, wc);
            else
                DrawRectangleRounded({c.x-cw/2+3, c.y-4, 5, 8}, 0.3f, 4, wc);
        }
    }
}

// ─────────────────────────────────────────────────────
//  Queue overlay bars (top corners)
// ─────────────────────────────────────────────────────
void Renderer::drawQueueBars(const Simulation& sim) {
    int nsQ = sim.nsQueue();
    int ewQ = sim.ewQueue();

    DrawRectangleRounded({8, 8, 88, 36}, 0.25f, 6, {20,23,32,215});
    Color nsC = sim.ctrl().signalColor(NORTH);
    DrawRectangle(12, 13, (int)std::min(nsQ*8.f, 76.f), 10, nsC);
    DrawText(TextFormat("NS: %d", nsQ), 12, 27, 10, COL_MUTED);

    DrawRectangleRounded({SIM_W-96.f, 8, 88, 36}, 0.25f, 6, {20,23,32,215});
    Color ewC = sim.ctrl().signalColor(EAST);
    DrawRectangle(SIM_W-92, 13, (int)std::min(ewQ*8.f, 76.f), 10, ewC);
    DrawText(TextFormat("EW: %d", ewQ), SIM_W-92, 27, 10, COL_MUTED);

    if (sim.ctrl().isOverride()) {
        float pulse = (sinf((float)GetTime() * 8.0f) + 1.0f) / 2.0f;
        unsigned char alpha = (unsigned char)(160 + pulse * 95);
        Color badgeBg = {180, 30, 30, alpha};
        float bw = 160, bh = 22;
        float bx = (SIM_W - bw) / 2.f;
        float by = 10;
        DrawRectangleRounded({bx, by, bw, bh}, 0.4f, 6, badgeBg);
        const char* etxt = "EMERGENCY OVERRIDE";
        int tw = MeasureText(etxt, 10);
        DrawText(etxt, (int)(bx + bw/2 - tw/2), (int)(by + 6), 10, {255,255,255,alpha});
    }
}

// ─────────────────────────────────────────────────────
//  Phase timer ring at intersection centre
// ─────────────────────────────────────────────────────
void Renderer::drawPhaseRing(const Simulation& sim) {
    if (sim.ctrl().isOverride()) {
        float pulse = (sinf((float)GetTime() * 6.0f) + 1.0f) / 2.0f;
        unsigned char alpha = (unsigned char)(120 + pulse * 135);
        DrawRing({CX,CY}, 9, 13, 0, 360, 32, {220,40,40,alpha});
        return;
    }
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

    DrawRectangleRounded({px-6, py-6, pw+12, WIN_H-py-4.f},
                         0.08f, 6, COL_PANEL);

    DrawText("Smart Traffic", (int)px, (int)py,    16, COL_TEXT);

    // Get current time
    std::time_t currTime = std::time(nullptr);
    char timeStr[64];
    std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&currTime));
    int timeW = MeasureText(timeStr, 14);
    DrawText(timeStr, (int)(px + pw - timeW), (int)py, 14, COL_TEXT);

    DrawText("Simulator",     (int)px, (int)(py+20),16, COL_TEXT);
    py += 50;

    const char* ml = sim.smartMode() ? "SMART  AI" : "FIXED TIMER";
    Color mbg  = sim.smartMode() ? Color{20,42,74,255}   : Color{42,32,10,255};
    Color mcol = sim.smartMode() ? Color{122,184,245,255}: Color{245,201,122,255};
    DrawRectangleRounded({px, py, pw, 26}, 0.4f, 6, mbg);
    int tw = MeasureText(ml, 12);
    DrawText(ml, (int)(px+pw/2-tw/2), (int)(py+7), 12, mcol);
    py += 36;

    if (sim.ctrl().isOverride()) {
        float pulse = (sinf((float)GetTime() * 8.0f) + 1.0f) / 2.0f;
        unsigned char alpha = (unsigned char)(180 + pulse * 75);
        Color eBg = {120, 20, 20, alpha};
        DrawRectangleRounded({px, py, pw, 26}, 0.4f, 6, eBg);
        const char* el = "AMBULANCE ACTIVE";
        int etw = MeasureText(el, 12);
        DrawText(el, (int)(px+pw/2-etw/2), (int)(py+7), 12, {255, 200, 200, alpha});
        py += 32;
    }

    const char* pn;
    Color pc;
    if      (sim.ctrl().phase()=="NS" && !sim.ctrl().isYellow()) { pn="NS  GREEN";  pc=COL_GREEN_SIG;  }
    else if (sim.ctrl().phase()=="NS" &&  sim.ctrl().isYellow()) { pn="NS  YELLOW"; pc=COL_YELLOW_SIG; }
    else if (sim.ctrl().phase()=="EW" && !sim.ctrl().isYellow()) { pn="EW  GREEN";  pc=COL_GREEN_SIG;  }
    else                                                           { pn="EW  YELLOW"; pc=COL_YELLOW_SIG; }
    DrawText("Phase", (int)px, (int)py, 10, COL_MUTED);
    DrawText(pn, (int)px, (int)(py+14), 14, pc);
    py += 38;

    float filled = (sim.ctrl().greenDur() > 0)
                   ? (sim.ctrl().timer() / sim.ctrl().greenDur()) * pw : 0;
    DrawRectangleRounded({px, py, pw, 8}, 0.5f, 4, {40,44,60,255});
    Color tbc = (sim.ctrl().phase()=="NS") ? COL_GREEN_SIG : COL_SENSOR;
    if (sim.ctrl().isOverride()) tbc = COL_AMBULANCE_CROSS;
    if (filled > 0) DrawRectangleRounded({px, py, filled, 8}, 0.5f, 4, tbc);
    py += 16;
    DrawText(TextFormat("%.1fs / %.0fs",
             sim.ctrl().timer(), sim.ctrl().greenDur()),
             (int)px, (int)py, 10, COL_MUTED);
    py += 24;

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
    py += 58;

    drawStatCard(px,       py, cw2, 50, "Ambulances",
                 TextFormat("%d", sim.ambulancesPassed()),
                 COL_AMBULANCE_CROSS);
    drawStatCard(px+cw2+8, py, cw2, 50, "Active",
                 sim.hasActiveAmbulance() ? "YES" : "NO",
                 sim.hasActiveAmbulance() ? COL_SIREN_RED : COL_MUTED);
    py += 64;

    DrawText("Queues", (int)px, (int)py, 10, COL_MUTED);
    py += 14;
    drawQueueBar(px, py,    pw, "N/S",
                 sim.nsQueue(), sim.ctrl().signalColor(NORTH));
    drawQueueBar(px, py+18, pw, "E/W",
                 sim.ewQueue(), sim.ctrl().signalColor(EAST));
    py += 48;

    DrawText("[S] Smart / Fixed",    (int)px, (int)py,     10, COL_MUTED);
    DrawText("[R] Reset",            (int)px, (int)(py+14),10, COL_MUTED);
    DrawText("[+]/[-] Speed",        (int)px, (int)(py+28),10, COL_MUTED);
    DrawText("[A] Spawn Ambulance",  (int)px, (int)(py+42),10, COL_MUTED);
    DrawText("[Q] Quit",             (int)px, (int)(py+56),10, COL_MUTED);

    py += 76;

    // ── Density Graph ────────────────────────────────
    DrawText("Density vs Time", (int)px, (int)py, 10, COL_MUTED);
    py += 14;

    float gh = 66;
    DrawRectangleRounded({px, py, pw, gh}, 0.2f, 4, COL_CARD);
    
    DrawRectangle((int)(px+5), (int)(py+5), 5, 5, COL_GREEN_SIG);
    DrawText("N/S", (int)(px+13), (int)(py+4), 8, COL_MUTED);

    DrawRectangle((int)(px+35), (int)(py+5), 5, 5, COL_SENSOR);
    DrawText("E/W", (int)(px+43), (int)(py+4), 8, COL_MUTED);

    const auto& nsHist = sim.getNsHistory();
    const auto& ewHist = sim.getEwHistory();

    int maxDensity = 5;
    for (int d : nsHist) if (d > maxDensity) maxDensity = d;
    for (int d : ewHist) if (d > maxDensity) maxDensity = d;

    float padYTop = 15.0f;
    float padYBot = 15.0f;
    float padXRight = 20.0f;
    float graphW = pw - padXRight;
    float graphH = gh - padYTop - padYBot;

    // Grid lines
    Color gridCol = {255, 255, 255, 20};
    DrawLineEx({px, py + padYTop}, {px + graphW, py + padYTop}, 1.0f, gridCol);
    DrawLineEx({px, py + padYTop + graphH/2}, {px + graphW, py + padYTop + graphH/2}, 1.0f, gridCol);
    DrawLineEx({px, py + padYTop + graphH}, {px + pw, py + padYTop + graphH}, 1.0f, gridCol);

    // Y-axis labels
    DrawText(TextFormat("%d", maxDensity), (int)(px + graphW + 4), (int)(py + padYTop - 4), 8, COL_MUTED);
    DrawText("0", (int)(px + graphW + 4), (int)(py + padYTop + graphH - 4), 8, COL_MUTED);

    // X-axis labels
    DrawText("-60s", (int)(px + 2),          (int)(py + gh - 12), 8, COL_MUTED);
    DrawText("-30s", (int)(px + graphW/2 - 10), (int)(py + gh - 12), 8, COL_MUTED);
    DrawText("Now",  (int)(px + graphW - 16),(int)(py + gh - 12), 8, COL_MUTED);

    auto drawLineGraph = [&](const std::deque<int>& hist, Color col) {
        if (hist.size() < 2) return;
        float xStep = graphW / 60.0f;
        float currentX = px + graphW - (hist.size() - 1) * xStep;

        for (size_t i = 0; i < hist.size() - 1; i++) {
            float y1 = py + padYTop + graphH - ((float)hist[i] / maxDensity) * graphH;
            float y2 = py + padYTop + graphH - ((float)hist[i+1] / maxDensity) * graphH;
            DrawLineEx({currentX, y1}, {currentX + xStep, y2}, 2.0f, col);
            DrawCircle((int)currentX, (int)y1, 1.0f, col);
            currentX += xStep;
        }
        if (!hist.empty()) {
            float yLast = py + padYTop + graphH - ((float)hist.back() / maxDensity) * graphH;
            DrawCircle((int)(px + graphW), (int)yLast, 1.0f, col);
        }
    };

    drawLineGraph(nsHist, COL_GREEN_SIG);
    drawLineGraph(ewHist, COL_SENSOR);
}

// ─────────────────────────────────────────────────────
//  Master draw — called once per frame
// ─────────────────────────────────────────────────────
void Renderer::draw(const Simulation& sim) {
    BeginTextureMode(rt_);
        drawRoad();
        drawLights(sim);
        drawCars(sim);
        drawQueueBars(sim);
        drawPhaseRing(sim);
    EndTextureMode();

    BeginDrawing();
        ClearBackground(COL_BG);

        DrawTexturePro(
            rt_.texture,
            {0, 0, (float)SIM_W, -(float)SIM_H},
            {8, 8, (float)SIM_W, (float)SIM_H},
            {0, 0}, 0, WHITE
        );
        DrawRectangleRoundedLinesEx({8,8,(float)SIM_W,(float)SIM_H},
                                    0.04f, 6, 1.0f, {42,47,66,255});

        drawPanel(sim);

        DrawRectangle(0, WIN_H-32, WIN_W, 32, {10,12,18,255});
        DrawText("Smart Traffic Simulator  |  C++17 + raylib",
                 12, WIN_H-20, 10, COL_MUTED);
        DrawText(TextFormat("FPS: %d", GetFPS()),
                 WIN_W-65, WIN_H-20, 10, COL_MUTED);

    EndDrawing();
}
