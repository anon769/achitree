#ifndef AI_LOGIC_H
#define AI_LOGIC_H

#include "types.h"
#include "raymath.h"
#include <queue>
#include <map>
#include <algorithm>

// vê se o nó está dentro da poça de água (ou mineral, já que usa mesma lógica)
inline bool IsPointInPuddleAI(Vector2 p, const Puddle& puddle){
    if (puddle.amount <= 0) return false;

    // normaliza o ponto em relação ao centro da poça (forma elíptica)
    Vector2 d = { (p.x - puddle.position.x) / (puddle.width / 2.0f),
                  (p.y - puddle.position.y) / (puddle.height / 2.0f) };

    // verifica se está dentro da elipse
    return (d.x * d.x + d.y * d.y) <= 1.0f;
}

// verifica se um recurso pode ser coletado em um node específico
inline bool IsResourceCollectableAtNode(int nodeIdx, ResourceType type, 
                                        const TreeResources& res, 
                                        const std::vector<Node>& nodes) 
{
    if (type == WATER) {
        for (auto& p : res.waterPuddles)
            if (p.amount > 0 && IsPointInPuddleAI(nodes[nodeIdx].position, p))
                return true;
    } 
    else if (type == MINERAL) {
        for (auto& m : res.minerals)
            if (m.amount > 0 && IsPointInPuddleAI(nodes[nodeIdx].position, m))
                return true;
    } 
    else if (type == LIGHT) {
        for (auto& lr : leafRegistry)
            if (lr.nodeIndex == nodeIdx && lr.active && lr.readyTimer <= 0)
                return true;
    }

    return false;
}

// decide qual o próximo nó que a unidade deve ir
inline int DecideNextNode(Unit& unit, const std::vector<Node>& nodes, const std::vector<Connection>& connections, const TreeResources& res){
    // acha o tronco (ponto base)
    int trunkIdx = 0;
    for (int i = 0; i < (int)nodes.size(); i++){
        if (nodes[i].type == TRUNK){
            trunkIdx = i;
            break;
        }
    }

    int target = trunkIdx;

    // -----------------------------
    // CASO: unidade já está carregando algo
    // -----------------------------
    if (unit.carrying != NONE){
        if (gSugarOrders > 0 && unit.carrying == WATER){
            unit.intent = SUGAR;

            float bestDist = 999999.0f;
            int bestLeaf = -1;

            for (auto& lr : leafRegistry){
                if (lr.active && lr.readyTimer <= 0){
                    float d = Vector2Distance(nodes[unit.startNodeIndex].position, nodes[lr.nodeIndex].position);
                    if (d < bestDist){
                        bestDist = d;
                        bestLeaf = lr.nodeIndex;
                    }
                }
            }

            if (bestLeaf != -1) target = bestLeaf;
            else target = trunkIdx;
        } else {
            unit.intent = NONE;
            target = trunkIdx;
        }

    } else {

        // -----------------------------
        // CASO: unidade está vazia (vai coletar recurso)
        // -----------------------------

        bool leafAvailable = false;
        for (auto& lr : leafRegistry){
            if (lr.active && lr.readyTimer <= 0){
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

        if (leafAvailable || puddleAvailable || mineralAvailable){
            NodeType goal = ROOT;
            ResourceType goalRes = NONE;

            // -----------------------------
            // SCORING DINÂMICO COM URGÊNCIA E LIMITE DE RECURSO
            // -----------------------------
            struct Candidate {
                int nodeIdx;
                float score;
                ResourceType type;
            };

            const float MAX_WATER = 100.0f;
            const float MAX_MINERAL = 100.0f;
            const float MAX_LIGHT = 100.0f;

            std::vector<Candidate> candidates;
            Vector2 unitPos = nodes[unit.startNodeIndex].position;

            // verifica todos os nodes e todos os recursos possíveis
            std::vector<ResourceType> resourceTypes = {WATER, MINERAL, LIGHT};

            for (auto resType : resourceTypes){
                NodeType t = (resType == LIGHT) ? LEAF : ROOT;

                for (int i = 0; i < (int)nodes.size(); i++){
                    if (nodes[i].type != t) continue;

                    if (!IsResourceCollectableAtNode(i, resType, res, nodes))
                        continue;

                    float dist = Vector2Distance(unitPos, nodes[i].position);

                    float fillRatio = 0.0f;
                    if (resType == WATER) fillRatio = res.waterLevel / MAX_WATER;
                    else if (resType == MINERAL) fillRatio = res.mineralLevel / MAX_MINERAL;
                    else if (resType == LIGHT) fillRatio = res.lightLevel / MAX_LIGHT;

                    float urgency = (fillRatio < 0.8f) ? (1.0f - fillRatio) : 0.0f;
                    if (urgency <= 0.0f) continue;

                    float score = urgency / (dist + 1.0f);
                    candidates.push_back({i, score, resType});
                }
            }

            // escolhe o node com maior score, independentemente do recurso
            if (!candidates.empty()){
                auto best = std::max_element(candidates.begin(), candidates.end(),
                                             [](const Candidate& a, const Candidate& b){ return a.score < b.score; });
                target = best->nodeIdx;
                goalRes = best->type;
            } else {
                target = trunkIdx;
                goalRes = NONE;
            }

            unit.intent = goalRes;
        }
    }

    if (unit.startNodeIndex == target)
        return unit.startNodeIndex;

    std::queue<int> q;
    q.push(unit.startNodeIndex);

    std::map<int, int> parent;
    parent[unit.startNodeIndex] = -1;

    bool found = false;

    while (!q.empty()){
        int curr = q.front();
        q.pop();

        if (curr == target){
            found = true;
            break;
        }

        for (auto& c : connections){
            int next = -1;

            if (c.from == curr) next = c.to;
            else if (c.to == curr) next = c.from;

            if (next != -1 && parent.find(next) == parent.end()){
                parent[next] = curr;
                q.push(next);
            }
        }

        for (auto& vc : virtualConnections){
            int next = -1;

            if (vc.from == curr) next = vc.to;
            else if (vc.to == curr) next = vc.from;

            if (next != -1 && parent.find(next) == parent.end()){
                parent[next] = curr;
                q.push(next);
            }
        }
    }

    if (found){
        int path = target;
        while (parent[path] != unit.startNodeIndex && parent[path] != -1){
            path = parent[path];
        }
        return path;
    }

    return unit.startNodeIndex;
}

#endif
