# Smart Traffic Simulator

A four-way intersection simulator with a real-time raylib window —
animated cars, adaptive traffic lights, queue bars, and a live stats panel.

---

## Project Structure

```
SmartTrafficSim/
├── include/
│   ├── types.h          ← constants, colours, enums, Snapshot
│   ├── vehicle.h        ← Car struct (movement, stop-line, gap)
│   ├── controller.h     ← SignalController (Smart AI / Fixed)
│   ├── simulation.h     ← Simulation (owns cars + controller)
│   └── renderer.h       ← Renderer (all raylib draw calls)
├── src/
│   ├── main.cpp         ← window init, input, game loop
│   ├── vehicle.cpp
│   ├── controller.cpp
│   ├── simulation.cpp
│   └── renderer.cpp
├── .vscode/
│   ├── tasks.json            ← Ctrl+Shift+B to build
│   └── c_cpp_properties.json ← IntelliSense paths
├── Makefile             ← make / make run / make clean
├── CMakeLists.txt       ← optional cmake build
└── README.md
```

---

## Prerequisites

Install raylib once:
```bash
brew install raylib
```

---

## Build & Run

### Quickest way — Terminal
```bash
cd ~/Downloads/SmartTrafficSim
make run
```

### VS Code
1. Open the `SmartTrafficSim/` folder in VS Code
2. Press **Ctrl+Shift+B** → Build
3. Open the integrated terminal and run `./SmartTrafficSim`

### Manual g++ (no make)
```bash
g++ -std=c++17 -O2 -I include \
    src/main.cpp src/vehicle.cpp src/controller.cpp \
    src/simulation.cpp src/renderer.cpp \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib -lraylib \
    -framework OpenGL -framework Cocoa \
    -framework IOKit -framework CoreVideo \
    -o SmartTrafficSim
./SmartTrafficSim
```

---

## Controls (keyboard)

| Key     | Action                        |
|---------|-------------------------------|
| `S`     | Toggle Smart AI ↔ Fixed timer |
| `R`     | Reset simulation              |
| `+`     | Increase simulation speed     |
| `-`     | Decrease simulation speed     |
| `Q`     | Quit                          |

---

## How the Smart AI works

The `SignalController` reads the current NS and EW queue lengths every tick.
When a green phase is about to end, it computes the next green duration:

```
next_green = clamp(BASE_GREEN + queue_serving*2 - queue_waiting,
                   MIN_GREEN, MAX_GREEN)
```

If the direction about to go green has more cars waiting, it gets a longer
green. This reduces total wait time under uneven traffic loads compared to
the fixed 20-second cycle.
```
