#include "include/simulation.h"
#include <iostream>
#include <cmath>

int main() {
    Simulation sim(true);
    int collisionCount = 0;
    
    // Simulate for a few minutes (e.g. 5 mins at 60fps)
    int frames = 60 * 60 * 5;
    
    for (int i = 0; i < frames; ++i) {
        sim.update(1.0f / 60.0f);
        
        auto cars = sim.cars();
        for (size_t a = 0; a < cars.size(); ++a) {
            for (size_t b = a + 1; b < cars.size(); ++b) {
                if (cars[a].gone || cars[b].gone) continue;
                
                float dx = cars[a].x - cars[b].x;
                float dy = cars[a].y - cars[b].y;
                float dist = std::sqrt(dx*dx + dy*dy);
                
                // If centers are within 10 pixels, they are severely clipped/collided
                if (dist < 10.0f) {
                    collisionCount++;
                }
            }
        }
    }
    
    if (collisionCount > 0) {
         std::cout << "Detected " << collisionCount << " total overlapping/colliding instances during headless test!\n";
    } else {
         std::cout << "No collisions detected!\n";
    }
    
    return 0;
}
