#ifndef AI_LOGIC_H
#define AI_LOGIC_H

#include "types.h"
#include "raymath.h"
#include <queue>
#include <map>

// ve se o nó ta dentro da poça de agua (ou mineral, já que usa mesma lógica)
inline bool IsPointInPuddleAI(Vector2 p, const Puddle& puddle) {
    if (puddle.amount <= 0) return false;

    // normaliza o ponto em relação ao centro da poça (forma elíptica)
    Vector2 d = { (p.x - puddle.position.x) / (puddle.width / 2.0f),
                  (p.y - puddle.position.y) / (puddle.height / 2.0f) };

    // verifica se está dentro da elipse
    return (d.x * d.x + d.y * d.y) <= 1.0f;
}

// decide qual o proximo nó que a unidade deve ir
inline int DecideNextNode(Unit& unit, const std::vector<Node>& nodes, const std::vector<Connection>& connections, const TreeResources& res) {
    
    // acha o tronco (ponto base)
    int trunkIdx = 0;
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].type == TRUNK) {
            trunkIdx = i;
            break;
        }
    }

    int target = trunkIdx;

    // -----------------------------
    // CASO: unidade já está carregando algo
    // -----------------------------
    if (unit.carrying != NONE) {

        // se precisa fazer açúcar e está com água → leva para folha
        if (gSugarOrders > 0 && unit.carrying == WATER) {
            unit.intent = SUGAR;

            float bestDist = 999999.0f;
            int bestLeaf = -1;

            // procura folha pronta mais próxima
            for (auto& lr : leafRegistry) {
                if (lr.active && lr.readyTimer <= 0) {
                    float d = Vector2Distance(nodes[unit.startNodeIndex].position,
                                              nodes[lr.nodeIndex].position);
                    if (d < bestDist) {
                        bestDist = d;
                        bestLeaf = lr.nodeIndex;
                    }
                }
            }

            if (bestLeaf != -1) target = bestLeaf;
            else target = trunkIdx; // fallback
        } else {
            // qualquer outro caso → volta pro tronco
            unit.intent = NONE;
            target = trunkIdx;
        }

    } else {

        // -----------------------------
        // CASO: unidade está vazia (vai coletar recurso)
        // -----------------------------

        bool leafAvailable = false;
        for (auto& lr : leafRegistry) {
            if (lr.active && lr.readyTimer <= 0) {
                leafAvailable = true;
                break;
            }
        }

        bool puddleAvailable = false;
        for (auto& p : res.waterPuddles)
            if (p.amount > 0) puddleAvailable = true;

        bool mineralAvailable = false;
        for (auto& m : res.minerals)
            if (m.amount > 0) mineralAvailable = true;

        // se existe algum recurso disponível
        if (leafAvailable || puddleAvailable || mineralAvailable) {

            NodeType goal = ROOT;
            ResourceType goalRes = NONE;
            
            // prioridade definida pelo jogador/sistema
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
                // decisão automática baseada nos níveis
                if (gSugarOrders > 0 && puddleAvailable) {
                    goal = ROOT; goalRes = WATER;

                } else if (res.mineralLevel < res.waterLevel &&
                           res.mineralLevel < res.lightLevel &&
                           mineralAvailable) {
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

            // procura o melhor nó do tipo desejado
            for (int i = 0; i < (int)nodes.size(); i++) {
                if (nodes[i].type == goal) {

                    bool prod = false;

                    // verifica se esse nó realmente produz recurso
                    if (goal == LEAF) {
                        for (auto& lr : leafRegistry)
                            if (lr.nodeIndex == i && lr.active && lr.readyTimer <= 0)
                                prod = true;
                    } else {
                        if (goalRes == WATER) {
                            for (auto& p : res.waterPuddles)
                                if (IsPointInPuddleAI(nodes[i].position, p))
                                    prod = true;

                        } else if (goalRes == MINERAL) {
                            for (auto& m : res.minerals)
                                if (IsPointInPuddleAI(nodes[i].position, m))
                                    prod = true;
                        }
                    }

                    // prioriza nós que realmente produzem
                    if (prod) {
                        if (!foundProductive) {
                            foundProductive = true;
                            bestDist = 999999.0f;
                        }
                    } else if (foundProductive) continue;

                    float d = Vector2Distance(nodes[unit.startNodeIndex].position,
                                              nodes[i].position);

                    if (d < bestDist) {
                        bestDist = d;
                        bestNodeIdx = i;
                    }
                }
            }

            if (bestNodeIdx != -1) target = bestNodeIdx;
        }
    }

    // se já está no alvo, não precisa andar
    if (unit.startNodeIndex == target)
        return unit.startNodeIndex;

    // -----------------------------
    // PATHFINDING (BFS simples)
    // -----------------------------
    std::queue<int> q;
    q.push(unit.startNodeIndex);

    std::map<int, int> parent;
    parent[unit.startNodeIndex] = -1;

    bool found = false;

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        if (curr == target) {
            found = true;
            break;
        }

        // conexões normais
        for (auto& c : connections) {
            int next = -1;

            if (c.from == curr) next = c.to;
            else if (c.to == curr) next = c.from;

            if (next != -1 && parent.find(next) == parent.end()) {
                parent[next] = curr;
                q.push(next);
            }
        }

        // conexões virtuais (atalhos, etc)
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

    // reconstrói o caminho (pega só o próximo passo)
    if (found) {
        int path = target;

        while (parent[path] != unit.startNodeIndex && parent[path] != -1) {
            path = parent[path];
        }

        return path;
    }

    // fallback: não encontrou caminho
    return unit.startNodeIndex;
}

#endif
