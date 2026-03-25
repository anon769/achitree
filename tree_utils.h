#ifndef TREE_UTILS_H
#define TREE_UTILS_H

#include "types.h"
#include "raymath.h"
#include <vector>

inline bool IsPointInPuddle(Vector2 p, const Puddle& puddle) {
    if (puddle.amount <= 0) return false;
    Vector2 d = { (p.x - puddle.position.x) / (puddle.width / 2.0f), (p.y - puddle.position.y) / (puddle.height / 2.0f) };
    return (d.x * d.x + d.y * d.y) <= 1.0f;
}

inline void HandleWeatherCycle(TreeResources& res, float dt, float centerX, float ground) {
    if ((res.rainTimer -= dt) <= 0) {
        if (!res.isRaining) {
            res.isRaining = true;
            res.rainTimer = (float)GetRandomValue(3, 6);
        } else {
            res.isRaining = false;
            res.rainTimer = 10.0f;
        }
    }
    
    if (res.isRaining && GetRandomValue(0, 100) < 5 && res.waterPuddles.size() < 20) {
        float pw = (float)GetRandomValue(70, 150);
        float spawnX = (float)GetRandomValue((int)centerX - 950, (int)centerX + 950);
        float spawnY = (GetRandomValue(1, 3) <= 2) ? 
                       (float)GetRandomValue((int)ground + 50, (int)ground + 450) : 
                       (float)GetRandomValue((int)ground + 451, (int)ground + 950);

        bool tooClose = false;
        for (const auto& p : res.waterPuddles) {
            if (Vector2Distance({spawnX, spawnY}, p.position) < 200.0f) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            res.waterPuddles.push_back({{spawnX, spawnY}, pw, pw * 0.5f, 100.0f});
        }
    }
}

#endif
