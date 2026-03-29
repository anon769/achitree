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
#include "world_config.h"

// ponteiro global para acessar unidades (evita passar pra todo lado)
inline std::vector<Unit>* gUnitsPtr = nullptr;

// registra o vetor de unidades no sistema de input
inline void RegisterUnits(std::vector<Unit>& units){
    gUnitsPtr = &units;
}

// controla zoom e movimento da câmera
inline void HandleCamera(Camera2D& camera, float centerX, float groundLevel){
    float wheel = GetMouseWheelMove();

    // zoom baseado no centro da tela
    if (wheel != 0){
        // define offset fixo no centro da tela
        camera.offset = { (float)GetScreenWidth()/2, (float)GetScreenHeight()/2 };

        // aplica zoom relativo ao centro
        camera.zoom = Clamp(camera.zoom + wheel * 0.12f, 0.4f, 3.0f);
    }

    // arrastar com botão direito
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        Vector2 delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }

    // limita a câmera dentro da área do jogo
    camera.target.x = Clamp(camera.target.x, centerX - MAP_LIMIT, centerX + MAP_LIMIT);
    camera.target.y = Clamp(camera.target.y, groundLevel - MAP_LIMIT, groundLevel + MAP_LIMIT);
}

// lida com construção da árvore e reset do jogo
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
    // reset completo do jogo
    if (IsKeyPressed(KEY_R)){
        nodes.clear();
        connections.clear();

        if (gUnitsPtr) gUnitsPtr->clear();

        // limpa todos sistemas globais
        leafRegistry.clear();
        fallingBranches.clear();
        virtualConnections.clear();
        gWormTrails.clear();
        gWorms.clear();
        gWormIdCounter = 0;

        // reseta recursos e estado global
        treeRes = {};
        gBudCount = 1.0f;
        gSugarCount = 0;
        gSugarOrders = 0;
        gCurrentFocus = FOCUS_NONE;

        // recria mundo inicial
        WorldConfig config = { (float)GetScreenWidth(), (float)GetScreenHeight(), groundLevel, centerX };
        SetupNewGame(treeRes, nodes, connections, config);
        InitLeafRegistry(nodes);

        // reinicia intro
        introTimer = 0.0f;
        introFinished = false;

        // --------- RESETA A CÂMERA ----------
        camera.offset = { (float)GetScreenWidth()/2, (float)GetScreenHeight()/2 };
        camera.target = { centerX, groundLevel };
        camera.rotation = 0.0f;
        camera.zoom = 3.0f; // zoom inicial da intro
        // ------------------------------------

        return;
    }

    // bloqueia construção durante a intro
    if (!introFinished) return;

    if (nodes.empty()) return;

    // clique esquerdo → construir
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){

        Vector2 mWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        
        // verifica se está dentro da área jogável
        if (mWorld.x >= centerX - MAP_LIMIT && mWorld.x <= centerX + MAP_LIMIT && mWorld.y >= groundLevel - MAP_LIMIT && mWorld.y <= groundLevel + MAP_LIMIT){
            bool holdingT = IsKeyDown(KEY_T);

            // define tipo baseado na posição e tecla
            bool clickingBelowGround = (mWorld.y > groundLevel + 2.0f);
            NodeType typeToBuild = clickingBelowGround ? ROOT : (holdingT ? TRUNK : LEAF);

            // custo base
            float budCost = (holdingT && !clickingBelowGround) ? 2.0f : 1.0f;
            
            // desconto se estiver em trilha de minhoca
            if (clickingBelowGround){
                for (const auto& tp : gWormTrails){
                    if (Vector2Distance(mWorld, tp.position) < 15.0f){
                        budCost = 0.5f;
                        break;
                    }
                }
            }

            // não constrói se não tiver recurso
            if (gBudCount < budCost) return;

            int closest = -1;
            float minDist = 250.0f;
            
            // procura nó válido mais próximo para conectar
            for (int i = 0; i < (int)nodes.size(); i++){
                bool canConnect = false;

                // regras de conexão por tipo
                if (typeToBuild == ROOT){
                    // ROOT só pode conectar a outro ROOT ou ao tronco inicial (índice 0)
                    if (nodes[i].type == ROOT || (nodes[i].type == TRUNK && i == 0)) canConnect = true;
                } else {
                    // TRUNK e LEAF não podem conectar a ROOT
                    if (nodes[i].type != ROOT) canConnect = true;
                }

                if (canConnect){
                    // considera deslocamento do vento na posição
                    Vector2 effectivePos = Vector2Add(nodes[i].position,
                        GetWindOffset(nodes[i].position, groundLevel, nodes[i].type, nodes));

                    float d = Vector2Distance(mWorld, effectivePos);

                    if (d < minDist){
                        minDist = d;
                        closest = i;
                    }
                }
            }

            // se encontrou um nó para conectar
            if (closest != -1){
                Vector2 effectiveParentPos = Vector2Add(nodes[closest].position,
                    GetWindOffset(nodes[closest].position, groundLevel, nodes[closest].type, nodes));

                float distToParent = Vector2Distance(mWorld, effectiveParentPos);

                // limite de distância (raiz pode ir mais longe)
                float maxAllowedDist = (clickingBelowGround) ? 150.0f : 45.0f;
                
                if (distToParent <= maxAllowedDist){

                    // consome recurso
                    gBudCount -= budCost;

                    // ajusta posição removendo efeito do vento
                    Vector2 newNodeStaticPos = Vector2Subtract(mWorld,
                        GetWindOffset(mWorld, groundLevel, typeToBuild, nodes));

                    // cria novo nó
                    nodes.push_back({newNodeStaticPos, typeToBuild});
                    int newNodeIdx = (int)nodes.size() - 1;

                    // conecta ao pai
                    connections.push_back({closest, newNodeIdx});
                    
                    // registra folha
                    if (typeToBuild == LEAF)
                        leafRegistry.push_back({newNodeIdx, true, 0.0f});

                    // se for tronco → converte caminho inteiro até a raiz em tronco
                    if (typeToBuild == TRUNK){

                        std::map<int, int> parentMap;
                        std::vector<int> q;

                        // BFS a partir da raiz (nó 0)
                        q.push_back(0);
                        parentMap[0] = -1;

                        int head = 0;
                        bool found = false;

                        while(head < (int)q.size()){
                            int curr = q[head++];

                            if(curr == newNodeIdx){
                                found = true;
                                break;
                            }

                            for(auto& c : connections){
                                int next = (c.from == curr) ? c.to :
                                           (c.to == curr ? c.from : -1);

                                if(next != -1 && parentMap.find(next) == parentMap.end()){
                                    parentMap[next] = curr;
                                    q.push_back(next);
                                }
                            }
                        }

                        // sobe o caminho e transforma em tronco
                        if(found){
                            int trace = newNodeIdx;

                            while(trace != -1){
                                if(nodes[trace].type != ROOT)
                                    nodes[trace].type = TRUNK;

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
