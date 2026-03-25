#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include "tree_manager.h"
#include "worm_manager.h"
#include "game_init.h"
#include <vector>
#include <map>

inline std::vector<Unit>* gUnitsPtr = nullptr;

inline void RegisterUnits(std::vector<Unit>& units) {
    gUnitsPtr = &units;
}

inline void HandleCamera(Camera2D& camera, float centerX, float groundLevel) {
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        camera.offset = GetMousePosition();
        camera.target = mouseWorldPos;
        camera.zoom = Clamp(camera.zoom + wheel * 0.12f, 0.4f, 3.0f);
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }
    camera.target.x = Clamp(camera.target.x, centerX - 1000.0f, centerX + 1000.0f);
    camera.target.y = Clamp(camera.target.y, groundLevel - 1000.0f, groundLevel + 1000.0f);
}

inline void HandleConstruction(
    std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    TreeResources& treeRes,
    Camera2D& camera,
    float groundLevel,
    float centerX,
    float& introTimer,
    bool& introFinished
) {
    if (IsKeyPressed(KEY_R)) {
        nodes.clear();
        connections.clear();
        if (gUnitsPtr) gUnitsPtr->clear();
        leafRegistry.clear();
        fallingBranches.clear();
        virtualConnections.clear();
        gWormTrails.clear();

        treeRes = {};
        gBudCount = 1.0f;
        gSugarCount = 0;
        gSugarOrders = 0;
        gCurrentFocus = FOCUS_NONE;

        WorldConfig config = { (float)GetScreenWidth(), (float)GetScreenHeight(), groundLevel, centerX };
        SetupNewGame(treeRes, nodes, connections, config);
        InitLeafRegistry(nodes);

        introTimer = 0.0f;
        introFinished = false;

        return;
    }

    if (!introFinished) return;

    if (nodes.empty()) return;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        
        if (mWorld.x >= centerX - 1000.0f && mWorld.x <= centerX + 1000.0f &&
            mWorld.y >= groundLevel - 1000.0f && mWorld.y <= groundLevel + 1000.0f) {
            
            bool holdingT = IsKeyDown(KEY_T);
            bool clickingBelowGround = (mWorld.y > groundLevel + 2.0f);
            NodeType typeToBuild = clickingBelowGround ? ROOT : (holdingT ? TRUNK : LEAF);

            float budCost = (holdingT && !clickingBelowGround) ? 2.0f : 1.0f;
            
            if (clickingBelowGround) {
                for (const auto& tp : gWormTrails) {
                    if (Vector2Distance(mWorld, tp.position) < 15.0f) {
                        budCost = 0.5f;
                        break;
                    }
                }
            }

            if (gBudCount < budCost) return;

            int closest = -1;
            float minDist = 250.0f;
            
            for (int i = 0; i < (int)nodes.size(); i++) {
                bool canConnect = false;
                if (typeToBuild == ROOT) {
                    if (nodes[i].type == ROOT || nodes[i].type == TRUNK) canConnect = true;
                } else {
                    if (nodes[i].type != ROOT) canConnect = true;
                }

                if (canConnect) {
                    Vector2 effectivePos = Vector2Add(nodes[i].position, GetWindOffset(nodes[i].position, groundLevel, nodes[i].type, nodes));
                    float d = Vector2Distance(mWorld, effectivePos);
                    if (d < minDist) { minDist = d; closest = i; }
                }
            }

            if (closest != -1) {
                Vector2 effectiveParentPos = Vector2Add(nodes[closest].position, GetWindOffset(nodes[closest].position, groundLevel, nodes[closest].type, nodes));
                float distToParent = Vector2Distance(mWorld, effectiveParentPos);
                float maxAllowedDist = (clickingBelowGround) ? 150.0f : 45.0f;
                
                if (distToParent <= maxAllowedDist) {
                    gBudCount -= budCost;
                    Vector2 newNodeStaticPos = Vector2Subtract(mWorld, GetWindOffset(mWorld, groundLevel, typeToBuild, nodes));
                    nodes.push_back({newNodeStaticPos, typeToBuild});
                    int newNodeIdx = (int)nodes.size() - 1;
                    connections.push_back({closest, newNodeIdx});
                    
                    if (typeToBuild == LEAF) leafRegistry.push_back({newNodeIdx, true, 0.0f});
                    if (typeToBuild == TRUNK) {
                        std::map<int, int> parentMap;
                        std::vector<int> q;
                        q.push_back(0);
                        parentMap[0] = -1;
                        int head = 0;
                        bool found = false;
                        while(head < (int)q.size()){
                            int curr = q[head++];
                            if(curr == newNodeIdx) { found = true; break; }
                            for(auto& c : connections){
                                int next = (c.from == curr) ? c.to : (c.to == curr ? c.from : -1);
                                if(next != -1 && parentMap.find(next) == parentMap.end()){
                                    parentMap[next] = curr;
                                    q.push_back(next);
                                }
                            }
                        }
                        if(found){
                            int trace = newNodeIdx;
                            while(trace != -1){
                                if(nodes[trace].type != ROOT) nodes[trace].type = TRUNK;
                                trace = parentMap[trace];
                            }
                        }
                    }
                }
            }
        }
    }
}

#endif
