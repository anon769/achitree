#ifndef GAME_INIT_H
#define GAME_INIT_H

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include <vector>

struct WorldConfig {
    float sw;
    float sh;
    float groundLevel;
    float centerX;
};

inline void GenerateWaterPuddles(TreeResources& res, const WorldConfig& config) {
    res.waterPuddles.push_back({{config.centerX, config.groundLevel + 100}, 120, 60, 100.0f});

    for(int i = 0; i < 5; i++) {
        float px, py;
        bool validPos = false;
        int attempts = 0;

        while (!validPos && attempts < 10) {
            px = (float)GetRandomValue(config.centerX - 950, config.centerX + 950);
            
            if (GetRandomValue(1, 3) <= 2) {
                py = (float)GetRandomValue(config.groundLevel + 50, config.groundLevel + 450);
            } else {
                py = (float)GetRandomValue(config.groundLevel + 451, config.groundLevel + 950);
            }

            validPos = true;
            for (const auto& p : res.waterPuddles) {
                if (Vector2Distance({px, py}, p.position) < 200.0f) {
                    validPos = false;
                    break;
                }
            }
            attempts++;
        }

        float pw = (float)GetRandomValue(80, 180);
        res.waterPuddles.push_back({{px, py}, pw, pw * 0.5f, 100.0f});
    }
}

inline void InitTreeStructure(std::vector<Node>& nodes, std::vector<Connection>& connections, const WorldConfig& config) {
    nodes.push_back({{config.centerX, config.groundLevel}, TRUNK});
    nodes.push_back({{config.centerX, config.groundLevel + 50}, ROOT});
    nodes.push_back({{config.centerX, config.groundLevel - 40}, LEAF});
    connections.push_back({0, 1});
    connections.push_back({0, 2});
}

inline void SetupNewGame(TreeResources& res, std::vector<Node>& nodes, std::vector<Connection>& connections, const WorldConfig& config) {
    GenerateWaterPuddles(res, config);
    InitTreeStructure(nodes, connections, config);
}

#endif
