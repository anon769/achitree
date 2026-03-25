#ifndef MINERAL_MANAGER_H
#define MINERAL_MANAGER_H
#include "types.h"
#include "raymath.h"

inline void UpdateMinerals(TreeResources& res, float groundLevel, float centerX, float dt) {
    if (GetRandomValue(0, 100) < 5 && res.minerals.size() < 25) {
        float px = (float)GetRandomValue(centerX - 950, centerX + 950);
        float py = (GetRandomValue(1, 3) <= 2) ? 
                   (float)GetRandomValue((int)groundLevel + 50, (int)groundLevel + 450) : 
                   (float)GetRandomValue((int)groundLevel + 451, (int)groundLevel + 950);
        
        float pw = (float)GetRandomValue(60, 120);
        bool tooClose = false;
        for (const auto& m : res.minerals) {
            if (Vector2Distance({px, py}, m.position) < 150.0f) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) res.minerals.push_back({{px, py}, pw, pw * 0.6f, 100.0f});
    }
}
#endif
