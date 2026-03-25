#ifndef WORM_MANAGER_H
#define WORM_MANAGER_H

#include "raylib.h"
#include <vector>
#include "raymath.h"

struct WormSegment {
    Vector2 position;
};

struct TrailPoint {
    Vector2 position;
    int wormId;
    float alpha;
    bool fading;
};

struct Worm {
    int id;
    std::vector<WormSegment> segments;
    Vector2 direction;
    float moveTimer;
    float speed;
};

inline std::vector<Worm> gWorms;
inline std::vector<TrailPoint> gWormTrails;
inline int gWormIdCounter = 0;

inline void UpdateWorms(float groundLevel, float dt) {
    float leftWall = GetScreenWidth()/2.0f - 1000.0f;
    float rightWall = GetScreenWidth()/2.0f + 1000.0f;
    float bottomWall = groundLevel + 1000.0f;

    if (gWorms.size() < 3 && GetRandomValue(0, 100) < 2) {
        Worm w;
        w.id = gWormIdCounter++;
        float startX, startY;
        int side = GetRandomValue(0, 2);

        if (side == 0) {
            startX = leftWall + 5.0f;
            startY = groundLevel + (float)GetRandomValue(50, 950);
            w.direction = { 1, 0 };
        } else if (side == 1) {
            startX = rightWall - 5.0f;
            startY = groundLevel + (float)GetRandomValue(50, 950);
            w.direction = { -1, 0 };
        } else {
            startX = (float)GetRandomValue(leftWall + 50, rightWall - 50);
            startY = bottomWall - 5.0f;
            w.direction = { 0, -1 };
        }

        for (int i = 0; i < 5; i++) {
            w.segments.push_back({{startX, startY}});
        }
        w.speed = (float)GetRandomValue(40, 80);
        w.moveTimer = 0;
        gWorms.push_back(w);
    }

    for (int i = (int)gWorms.size() - 1; i >= 0; i--) {
        Worm& w = gWorms[i];
        w.moveTimer += dt;

        if (GetRandomValue(0, 100) < 5) {
            float currentAngle = atan2f(w.direction.y, w.direction.x);
            currentAngle += (float)GetRandomValue(-30, 30) * DEG2RAD;
            w.direction = { cosf(currentAngle), sinf(currentAngle) };
        }

        Vector2 nextPos = Vector2Add(w.segments[0].position, Vector2Scale(w.direction, w.speed * dt));

        if (nextPos.y < groundLevel + 10.0f) {
            w.direction.y = fabsf(w.direction.y);
            nextPos.y = groundLevel + 10.0f;
        }
        
        if (nextPos.x <= leftWall || nextPos.x >= rightWall || nextPos.y >= bottomWall) {
            int currentId = w.id;
            gWorms.erase(gWorms.begin() + i);
            for (auto& tp : gWormTrails) {
                if (tp.wormId == currentId) tp.fading = true;
            }
            continue;
        }

        if (w.moveTimer > 0.05f) {
            gWormTrails.push_back({w.segments[0].position, w.id, 180.0f, false});
            for (int j = (int)w.segments.size() - 1; j > 0; j--) {
                w.segments[j].position = w.segments[j - 1].position;
            }
            w.moveTimer = 0;
        }
        w.segments[0].position = nextPos;
    }

    for (int i = (int)gWormTrails.size() - 1; i >= 0; i--) {
        if (gWormTrails[i].fading) {
            gWormTrails[i].alpha -= 100.0f * dt;
            if (gWormTrails[i].alpha <= 0) {
                gWormTrails.erase(gWormTrails.begin() + i);
            }
        }
    }
}

inline void DrawWormTrails() {
    for (const auto& tp : gWormTrails) {
        DrawCircleV(tp.position, 2.5f, {40, 30, 20, (unsigned char)tp.alpha});
    }
}

inline void DrawWorms() {
    for (const auto& w : gWorms) {
        for (int i = 0; i < (int)w.segments.size(); i++) {
            DrawCircleV(w.segments[i].position, 3.5f, {255, 180, 160, 255});
        }
    }
}

#endif
