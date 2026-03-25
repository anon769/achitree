#ifndef TREE_MANAGER_H
#define TREE_MANAGER_H

#include "tree_utils.h"
#include "tree_entities.h"
#include "tree_view.h"
#include "environment.h"
#include "worm_manager.h"
#include "mineral_manager.h"
#include <map>
#include <functional>
#include <set>

inline void UpdateEcosystem(std::vector<Unit>& units, std::vector<Node>& nodes, std::vector<Connection>& connections, TreeResources& res, int sw, int sh, Camera2D camera) {
    float dt = GetFrameTime();
    res.gameTime += dt;
    float difficultyMultiplierLight = fminf(1.0f + (res.gameTime / 120.0f), 5.0f);
    float nodeCountBonus = nodes.size() * 0.05f;
    float difficultyMultiplierWaterMineral = fminf(1.0f + nodeCountBonus, 5.0f);
    float centerX = sw / 2.0f;
    float ground = sh * 0.65f;
    
    UpdateWeather(dt);
    UpdateWorms(ground, dt);
    UpdateMinerals(res, ground, centerX, dt);
    HandleWeatherCycle(res, dt, centerX, ground);
    
    if (IsKeyPressed(KEY_B)) {
        if (gSugarCount >= 1 && res.mineralLevel >= 20.0f) {
            gSugarCount -= 1;
            res.mineralLevel -= 20.0f;
            gBudCount += 1;
        }
    }

    for (auto& lr : leafRegistry) {
        if (lr.readyTimer > 0) lr.readyTimer -= dt;
    }
    
    res.waterLevel = Clamp(res.waterLevel - (res.consumptionRate * difficultyMultiplierWaterMineral) * dt, 0, res.maxLevel);
    res.lightLevel = Clamp(res.lightLevel - (res.consumptionRate * difficultyMultiplierLight) * dt, 0, res.maxLevel);
    res.mineralLevel = Clamp(res.mineralLevel - (res.consumptionRate * difficultyMultiplierWaterMineral) * dt, 0, res.maxLevel);
    
    float healthChange = (res.waterLevel > 0 && res.lightLevel > 0 && res.mineralLevel > 0) ? 1.5f : -8.0f;
    res.treeHealth = Clamp(res.treeHealth + healthChange * dt, 0, res.maxLevel);

    if (gWeather.windStrength >= 0.42f) {
        std::map<int, std::vector<int>> adj;
        for (const auto& c : connections) {
            adj[c.from].push_back(c.to);
            adj[c.to].push_back(c.from);
        }

        std::function<bool(int, int, int)> checkSequence = [&](int curr, int prev, int depth) -> bool {
            if (nodes[curr].type == TRUNK || nodes[curr].type == ROOT) return true;
            if (depth >= 3) return false; 
            if (adj.find(curr) == adj.end()) return false;
            for (int neighbor : adj[curr]) {
                if (neighbor != prev) {
                    if (checkSequence(neighbor, curr, depth + 1)) return true;
                }
            }
            return false;
        };

        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 0; i < (int)connections.size(); i++) {
                int n3 = connections[i].from;
                int n4 = connections[i].to;

                if (nodes[n3].type != ROOT && nodes[n3].type != TRUNK && nodes[n4].type != ROOT && nodes[n4].type != TRUNK) {
                    if (!checkSequence(n3, n4, 1)) {
                        std::set<int> nodesToRemove;
                        std::function<void(int, int)> findSubtree = [&](int curr, int prev) {
                            if (nodes[curr].type == TRUNK || nodes[curr].type == ROOT) return;
                            nodesToRemove.insert(curr);
                            for (int j = 0; j < (int)connections.size(); j++) {
                                int other = -1;
                                if (connections[j].from == curr && connections[j].to != prev) other = connections[j].to;
                                else if (connections[j].to == curr && connections[j].from != prev) other = connections[j].from;
                                if (other != -1) findSubtree(other, curr);
                            }
                        };
                        
                        findSubtree(n4, n3);

                        for (int j = (int)connections.size() - 1; j >= 0; j--) {
                            if (nodesToRemove.count(connections[j].from) || nodesToRemove.count(connections[j].to)) {
                                Vector2 p1 = Vector2Add(nodes[connections[j].from].position, GetWindOffset(nodes[connections[j].from].position, ground, nodes[connections[j].from].type, nodes));
                                Vector2 p2 = Vector2Add(nodes[connections[j].to].position, GetWindOffset(nodes[connections[j].to].position, ground, nodes[connections[j].to].type, nodes));
                                fallingBranches.push_back({p1, p2, { (float)GetRandomValue(-60, 60), 150.0f }, 1.0f, 3.0f, 3.0f});
                                connections.erase(connections.begin() + j);
                            }
                        }

                        for (auto& unit : units) {
                            if (nodesToRemove.count(unit.startNodeIndex) || nodesToRemove.count(unit.targetNodeIndex)) {
                                unit.startNodeIndex = n3;
                                unit.targetNodeIndex = n3;
                                unit.progress = 0.0f;
                                unit.currentPos = Vector2Add(nodes[n3].position, GetWindOffset(nodes[n3].position, ground, nodes[n3].type, nodes));
                            }
                        }

                        for (int nodeIdx : nodesToRemove) {
                            for (int j = (int)leafRegistry.size() - 1; j >= 0; j--) {
                                if (leafRegistry[j].nodeIndex == nodeIdx) leafRegistry.erase(leafRegistry.begin() + j);
                            }
                            nodes[nodeIdx].position = {-9999, -9999};
                            nodes[nodeIdx].type = BRANCH;
                        }
                        
                        changed = true;
                        break; 
                    }
                }
            }
        }
    }

    for (int i = (int)fallingBranches.size() - 1; i >= 0; i--) {
        fallingBranches[i].p1 = Vector2Add(fallingBranches[i].p1, Vector2Scale(fallingBranches[i].velocity, dt));
        fallingBranches[i].p2 = Vector2Add(fallingBranches[i].p2, Vector2Scale(fallingBranches[i].velocity, dt));
        fallingBranches[i].velocity.y += 600.0f * dt;
        if (fallingBranches[i].p1.y >= ground || fallingBranches[i].p2.y >= ground) fallingBranches[i].velocity = {0, 0};
        fallingBranches[i].lifeTimer -= dt;
        if (fallingBranches[i].lifeTimer <= 0) {
            fallingBranches[i].alpha -= dt * 0.5f;
            if (fallingBranches[i].alpha <= 0) fallingBranches.erase(fallingBranches.begin() + i);
        }
    }
    
    virtualConnections.clear();
    for(int i = 0; i < (int)nodes.size(); i++) {
        for(int j = i + 1; j < (int)nodes.size(); j++) {
            if(nodes[i].type == ROOT && nodes[j].type == ROOT) {
                if(Vector2Distance(nodes[i].position, nodes[j].position) < 100.0f) {
                    bool exists = false;
                    for(auto& c : connections) {
                        if((c.from == i && c.to == j) || (c.from == j && c.to == i)) { exists = true; break; }
                    }
                    if(!exists) virtualConnections.push_back({i, j});
                }
            }
        }
    }
    UpdateUnits(units, nodes, connections, res, ground, dt);
    for (int i = (int)res.waterPuddles.size() - 1; i >= 0; i--) if (res.waterPuddles[i].amount <= 0) res.waterPuddles.erase(res.waterPuddles.begin() + i);
    for (int i = (int)res.minerals.size() - 1; i >= 0; i--) if (res.minerals[i].amount <= 0) res.minerals.erase(res.minerals.begin() + i);
}

#endif
