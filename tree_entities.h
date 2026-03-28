#ifndef TREE_ENTITIES_H
#define TREE_ENTITIES_H

#include "types.h"
#include "tree_utils.h"
#include "ai_logic.h"
#include "unit_manager.h"
#include "environment.h"

// atualiza todas as unidades (movimento + lógica + coleta/entrega)
inline void UpdateUnits(std::vector<Unit>& units, std::vector<Node>& nodes, const std::vector<Connection>& connections, TreeResources& res, float ground, float dt){

    // input manual (spawn, controle, etc)
    HandleUnitInput(units, nodes[0].position, res);

    for (auto& unit : units){

        // progresso de movimento entre nós
        float dist = Vector2Distance(nodes[unit.startNodeIndex].position,
                                     nodes[unit.targetNodeIndex].position);

        unit.progress = (dist > 0)
            ? Clamp(unit.progress + (unit.speed / dist) * dt, 0, 1.0f)
            : 1.0f;
        
        // chegou no destino
        if (unit.progress >= 1.0f){

            unit.progress = 0.0f;
            unit.startNodeIndex = unit.targetNodeIndex;

            Node& currNode = nodes[unit.startNodeIndex];
            
            // decide próximo destino (usado também pra lógica local)
            int finalDestination = DecideNextNode(unit, nodes, connections, res);

            // -----------------------------
            // INTERAÇÃO COM FOLHAS
            // -----------------------------
            if (currNode.type == LEAF && (unit.intent == LIGHT || unit.intent == SUGAR)){
                for (auto& lr : leafRegistry){
                    if (lr.nodeIndex == unit.startNodeIndex){
                        if (lr.active && lr.readyTimer <= 0){
                            // transforma água em açúcar
                            if (unit.carrying == WATER && unit.intent == SUGAR){
                                unit.carrying = SUGAR;
                                unit.intent = NONE;
                                lr.readyTimer = lr.maxCooldown;

                            // coleta luz
                            } else if (unit.carrying == NONE && unit.intent == LIGHT){
                                unit.carrying = LIGHT; 
                                unit.intent = NONE;
                                lr.readyTimer = lr.maxCooldown;

                                // bônus baseado na altura da folha
                                float height = ground - nodes[unit.startNodeIndex].position.y;
                                unit.carryBonus = 1.0f + (height / 500.0f);
                            }
                        }
                    }
                }

            // -----------------------------
            // INTERAÇÃO COM TRONCO (BASE)
            // -----------------------------
            } else if (unit.startNodeIndex == 0){
                // entrega açúcar (objetivo principal)
                if (unit.carrying == SUGAR){
                    gSugarCount++;
                    if (gSugarOrders > 0) gSugarOrders--;

                    unit.carrying = NONE;
                    unit.intent = NONE;

                // entrega recursos básicos
                } else if (unit.carrying == WATER || unit.carrying == LIGHT || unit.carrying == MINERAL){
                    // água pode ser desviada pra produção de açúcar
                    bool shouldPassThrough = (gSugarOrders > 0 && unit.carrying == WATER);

                    if (!shouldPassThrough){
                        if (unit.carrying == WATER)
                            res.waterLevel = fminf(res.waterLevel + 20.0f, res.maxLevel);

                        if (unit.carrying == LIGHT){
                            float lightAmount = 20.0f * unit.carryBonus;
                            res.lightLevel = fminf(res.lightLevel + lightAmount, res.maxLevel);
                            unit.carryBonus = 1.0f;
                        }

                        if (unit.carrying == MINERAL)
                            res.mineralLevel = fminf(res.mineralLevel + 20.0f, res.maxLevel);

                        unit.carrying = NONE;
                        unit.intent = NONE;
                    }
                }

            // -----------------------------
            // INTERAÇÃO COM RAÍZES (COLETA)
            // -----------------------------
            } else if (currNode.type == ROOT &&
                       unit.carrying == NONE &&
                       finalDestination == unit.startNodeIndex){

                // coleta água
                if (unit.intent == WATER){
                    for (auto& p : res.waterPuddles){
                        if (IsPointInPuddle(currNode.position, p)){
                            unit.carrying = WATER;
                            p.amount -= 20.0f;
                            break;
                        }
                    }

                // coleta mineral
                } else if (unit.intent == MINERAL){
                    for (auto& m : res.minerals){
                        if (IsPointInPuddle(currNode.position, m)){
                            unit.carrying = MINERAL;
                            m.amount -= 20.0f;
                            break;
                        }
                    }
                }
            }
            
            // define próximo nó de destino
            unit.targetNodeIndex = DecideNextNode(unit, nodes, connections, res);
        }

        // -----------------------------
        // ATUALIZA POSIÇÃO VISUAL (com vento)
        // -----------------------------
        Vector2 startPos = Vector2Add(
            nodes[unit.startNodeIndex].position,
            GetWindOffset(nodes[unit.startNodeIndex].position,
                          ground,
                          nodes[unit.startNodeIndex].type,
                          nodes)
        );

        Vector2 endPos = Vector2Add(
            nodes[unit.targetNodeIndex].position,
            GetWindOffset(nodes[unit.targetNodeIndex].position,
                          ground,
                          nodes[unit.targetNodeIndex].type,
                          nodes)
        );

        // interpola posição atual
        unit.currentPos = Vector2Lerp(startPos, endPos, unit.progress);
    }
}

#endif
