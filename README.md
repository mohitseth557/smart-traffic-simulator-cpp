# 🚦 Smart Traffic Simulator

A highly dynamic, four-way traffic intersection simulator written natively in **C++17** and **Raylib**. This project models modern urban traffic problems and demonstrates dynamic queue resolution, emergency vehicle routing, and real-time density visualization.

## ✨ Key Features
- **Smart AI Controller**: The traffic signal dynamically adjusts its green-light frequency based on the physical size of the North/South and East/West vehicle queues to prevent gridlock.
- **Emergency Priority Overrides**: Ambulances seamlessly snap the traffic light states to enforce priority right-of-way and clear out intersecting vehicles.
- **Dynamic Density Graphs**: A live rolling-window chart visualizes traffic density over the last 60 seconds on both axes.
- **Advanced Pathing**: Vehicles utilize smooth mathematical Bezier curves with physical yielding logic for unprotected left and right turns.
- **Native Audio Engine**: Immersive synthetic soundscapes including UI clicks, looping Doppler sirens for ambulances, and frustration honks when traffic piles up.

## 🛠️ Tech Stack
- **Language**: C++17
- **Graphics & Audio API**: [Raylib](https://www.raylib.com/)

## 🚀 How to Run Locally (macOS)
1. Ensure you have the `raylib` library installed via Homebrew:
   ```bash
   brew install raylib
