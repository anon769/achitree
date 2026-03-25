#ifndef TREE_ENTITIES_H
#define TREE_ENTITIES_H

#include "types.h"
#include "tree_utils.h"
#include "ai_logic.h"
#include "unit_manager.h"
#include "environment.h"

inline void UpdateUnits(std::vector<Unit>& units, std::vector<Node>& nodes, const std::vector<Connection>& connections, TreeResources& res, float ground, float dt) {
    HandleUnitInput(units, nodes[0].position, res);

    for (auto& unit : units) {
        float dist = Vector2Distance(nodes[unit.startNodeIndex].position, nodes[unit.targetNodeIndex].position);
        unit.progress = (dist > 0) ? Clamp(unit.progress + (unit.speed / dist) * dt, 0, 1.0f) : 1.0f;
        
        if (unit.progress >= 1.0f) {
            unit.progress = 0.0f;
            unit.startNodeIndex = unit.targetNodeIndex;
            Node& currNode = nodes[unit.startNodeIndex];
            
            int finalDestination = DecideNextNode(unit, nodes, connections, res);

            if (currNode.type == LEAF && (unit.intent == LIGHT || unit.intent == SUGAR)) {
                for (auto& lr : leafRegistry) {
                    if (lr.nodeIndex == unit.startNodeIndex) {
                        if (lr.active && lr.readyTimer <= 0) {
                            if (unit.carrying == WATER && unit.intent == SUGAR) {
                                unit.carrying = SUGAR;
                                unit.intent = NONE;
                                lr.readyTimer = lr.maxCooldown;
                            } else if (unit.carrying == NONE && unit.intent == LIGHT) {
                                unit.carrying = LIGHT; 
                                unit.intent = NONE;
                                lr.readyTimer = lr.maxCooldown;

                                float height = ground - nodes[unit.startNodeIndex].position.y;
                                unit.carryBonus = 1.0f + (height / 500.0f);
                            }
                        }
                    }
                }
            } else if (unit.startNodeIndex == 0) {
                if (unit.carrying == SUGAR) {
                    gSugarCount++;
                    if (gSugarOrders > 0) gSugarOrders--;
                    unit.carrying = NONE;
                    unit.intent = NONE;
                } else if (unit.carrying == WATER || unit.carrying == LIGHT || unit.carrying == MINERAL) {
                    bool shouldPassThrough = (gSugarOrders > 0 && unit.carrying == WATER);
                    if (!shouldPassThrough) {
                        if (unit.carrying == WATER) res.waterLevel = fminf(res.waterLevel + 20.0f, res.maxLevel);
                        if (unit.carrying == LIGHT) {
                            float lightAmount = 20.0f * unit.carryBonus;
                            res.lightLevel = fminf(res.lightLevel + lightAmount, res.maxLevel);
                            unit.carryBonus = 1.0f;
                        }
                        if (unit.carrying == MINERAL) res.mineralLevel = fminf(res.mineralLevel + 20.0f, res.maxLevel);
                        unit.carrying = NONE;
                        unit.intent = NONE;
                    }
                }
            } else if (currNode.type == ROOT && unit.carrying == NONE && finalDestination == unit.startNodeIndex) {
                if (unit.intent == WATER) {
                    for (auto& p : res.waterPuddles) {
                        if (IsPointInPuddle(currNode.position, p)) {
                            unit.carrying = WATER; p.amount -= 20.0f; break;
                        }
                    }
                } else if (unit.intent == MINERAL) {
                    for (auto& m : res.minerals) {
                        if (IsPointInPuddle(currNode.position, m)) {
                            unit.carrying = MINERAL; m.amount -= 20.0f; break;
                        }
                    }
                }
            }
            
            unit.targetNodeIndex = DecideNextNode(unit, nodes, connections, res);
        }
        Vector2 startPos = Vector2Add(nodes[unit.startNodeIndex].position, GetWindOffset(nodes[unit.startNodeIndex].position, ground, nodes[unit.startNodeIndex].type, nodes));
        Vector2 endPos = Vector2Add(nodes[unit.targetNodeIndex].position, GetWindOffset(nodes[unit.targetNodeIndex].position, ground, nodes[unit.targetNodeIndex].type, nodes));
        unit.currentPos = Vector2Lerp(startPos, endPos, unit.progress);
    }
}

#endif
