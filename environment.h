#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "raylib.h"
#include <math.h>
#include <vector>
#include "types.h"
#include "raymath.h"

struct WeatherSystem {
    float windTime;
    float windStrength;
    float targetWind;
    float windChangeTimer;
};

inline WeatherSystem gWeather = { 0.0f, 0.1f, 0.2f, 0.0f };

inline void UpdateWeather(float dt) {
    gWeather.windTime += dt;
    gWeather.windChangeTimer -= dt;
    
    if (gWeather.windChangeTimer <= 0) {
        gWeather.targetWind = (float)GetRandomValue(0, 60) / 100.0f;
        gWeather.windChangeTimer = (float)GetRandomValue(5, 12);
    }
    
    gWeather.windStrength = Lerp(gWeather.windStrength, gWeather.targetWind, dt * 0.3f);
}

inline Vector2 GetWindOffset(Vector2 pos, float heightThreshold, NodeType type, const std::vector<Node>& nodes) {
    if (pos.y > heightThreshold + 5.0f || type == ROOT || type == TRUNK) return { 0, 0 };
    
    float minTrunkDist = 9999.0f;
    for (const auto& node : nodes) {
        if (node.type == TRUNK) {
            float d = Vector2Distance(pos, node.position);
            if (d < minTrunkDist) minTrunkDist = d;
        }
    }

    float distanceFactor = minTrunkDist * 0.12f;
    float speed = 0.5f + (gWeather.windStrength * 4.0f);
    float amplitude = gWeather.windStrength * distanceFactor;
    
    if (amplitude > 25.0f) amplitude = 25.0f;
    
    float sway = sinf(gWeather.windTime * speed + (pos.x * 0.02f)) * amplitude;
    return { sway, 0 };
}

#endif
