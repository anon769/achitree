#ifndef AI_LOGIC_H
#define AI_LOGIC_H

#include "types.h"
#include "raymath.h"
#include <queue>
#include <map>

inline bool IsPointInPuddleAI(Vector2 p, const Puddle& puddle) {
    if (puddle.amount <= 0) return false;
    Vector2 d = { (p.x - puddle.position.x) / (puddle.width / 2.0f), (p.y - puddle.position.y) / (puddle.height / 2.0f) };
    return (d.x * d.x + d.y * d.y) <= 1.0f;
}

inline int DecideNextNode(Unit& unit, const std::vector<Node>& nodes, const std::vector<Connection>& connections, const TreeResources& res) {
    int trunkIdx = 0;
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].type == TRUNK) {
            trunkIdx = i;
            break;
        }
    }

    int target = trunkIdx;

    if (unit.carrying != NONE) {
        if (gSugarOrders > 0 && unit.carrying == WATER) {
            unit.intent = SUGAR;
            float bestDist = 999999.0f;
            int bestLeaf = -1;
            for (auto& lr : leafRegistry) {
                if (lr.active && lr.readyTimer <= 0) {
                    float d = Vector2Distance(nodes[unit.startNodeIndex].position, nodes[lr.nodeIndex].position);
                    if (d < bestDist) { bestDist = d; bestLeaf = lr.nodeIndex; }
                }
            }
            if (bestLeaf != -1) target = bestLeaf;
            else target = trunkIdx;
        } else {
            unit.intent = NONE;
            target = trunkIdx;
        }
    } else {
        bool leafAvailable = false;
        for (auto& lr : leafRegistry) {
            if (lr.active && lr.readyTimer <= 0) {
                leafAvailable = true;
                break;
            }
        }

        bool puddleAvailable = false;
        for (auto& p : res.waterPuddles) if (p.amount > 0) puddleAvailable = true;

        bool mineralAvailable = false;
        for (auto& m : res.minerals) if (m.amount > 0) mineralAvailable = true;

        if (leafAvailable || puddleAvailable || mineralAvailable) {
            NodeType goal = ROOT;
            ResourceType goalRes = NONE;
            
            if (gCurrentFocus == FOCUS_WATER) {
                goal = puddleAvailable ? ROOT : TRUNK;
                goalRes = WATER;
            } else if (gCurrentFocus == FOCUS_MINERAL) {
                goal = mineralAvailable ? ROOT : TRUNK;
                goalRes = MINERAL;
            } else if (gCurrentFocus == FOCUS_LIGHT) {
                goal = leafAvailable ? LEAF : TRUNK;
                goalRes = LIGHT;
            } else {
                if (gSugarOrders > 0 && puddleAvailable) {
                    goal = ROOT; goalRes = WATER;
                } else if (res.mineralLevel < res.waterLevel && res.mineralLevel < res.lightLevel && mineralAvailable) {
                    goal = ROOT; goalRes = MINERAL;
                } else if (res.waterLevel < res.lightLevel && puddleAvailable) {
                    goal = ROOT; goalRes = WATER;
                } else if (leafAvailable) {
                    goal = LEAF; goalRes = LIGHT;
                } else {
                    goal = ROOT; goalRes = WATER;
                }
            }

            unit.intent = goalRes;

            float bestDist = 999999.0f;
            int bestNodeIdx = -1;
            bool foundProductive = false;

            for (int i = 0; i < (int)nodes.size(); i++) {
                if (nodes[i].type == goal) {
                    bool prod = false;
                    if (goal == LEAF) {
                        for (auto& lr : leafRegistry) if (lr.nodeIndex == i && lr.active && lr.readyTimer <= 0) prod = true;
                    } else {
                        if (goalRes == WATER) {
                            for (auto& p : res.waterPuddles) if (IsPointInPuddleAI(nodes[i].position, p)) prod = true;
                        } else if (goalRes == MINERAL) {
                            for (auto& m : res.minerals) if (IsPointInPuddleAI(nodes[i].position, m)) prod = true;
                        }
                    }

                    if (prod) {
                        if (!foundProductive) { foundProductive = true; bestDist = 999999.0f; }
                    } else if (foundProductive) continue;

                    float d = Vector2Distance(nodes[unit.startNodeIndex].position, nodes[i].position);
                    if (d < bestDist) { bestDist = d; bestNodeIdx = i; }
                }
            }
            if (bestNodeIdx != -1) target = bestNodeIdx;
        }
    }

    if (unit.startNodeIndex == target) return unit.startNodeIndex;

    std::queue<int> q;
    q.push(unit.startNodeIndex);
    std::map<int, int> parent;
    parent[unit.startNodeIndex] = -1;

    bool found = false;
    while (!q.empty()) {
        int curr = q.front();
        q.pop();
        if (curr == target) { found = true; break; }

        for (auto& c : connections) {
            int next = -1;
            if (c.from == curr) next = c.to;
            else if (c.to == curr) next = c.from;
            if (next != -1 && parent.find(next) == parent.end()) {
                parent[next] = curr;
                q.push(next);
            }
        }

        for (auto& vc : virtualConnections) {
            int next = -1;
            if (vc.from == curr) next = vc.to;
            else if (vc.to == curr) next = vc.from;
            if (next != -1 && parent.find(next) == parent.end()) {
                parent[next] = curr;
                q.push(next);
            }
        }
    }

    if (found) {
        int path = target;
        while (parent[path] != unit.startNodeIndex && parent[path] != -1) {
            path = parent[path];
        }
        return path;
    }

    return unit.startNodeIndex;
}

#endif
