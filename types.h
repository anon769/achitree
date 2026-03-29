#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <vector>

// tipos de nó da árvore
enum NodeType { TRUNK, BRANCH, ROOT, LEAF };

// tipos de recurso que unidades podem carregar
enum ResourceType { NONE, WATER, LIGHT, MINERAL, SUGAR };

// foco de coleta de recursos
enum CollectionFocus { FOCUS_NONE, FOCUS_WATER, FOCUS_LIGHT, FOCUS_MINERAL };

// nó da árvore
struct Node {
    Vector2 position; // posição na tela/mundo
    NodeType type;    // tipo do nó
};

// conexão entre dois nós
struct Connection {
    int from;
    int to;
};

// representa poças de água ou minerais
struct Puddle {
    Vector2 position;
    float width;
    float height;
    float amount; // quantidade de recurso
};

// unidade que se move e coleta recursos
struct Unit {
    Vector2 currentPos;      // posição atual
    int startNodeIndex;      // nó inicial
    int targetNodeIndex;     // nó alvo
    float progress;          // progresso do movimento
    float speed;             // velocidade
    ResourceType carrying = NONE; // recurso carregado
    ResourceType intent = NONE;   // intenção de coleta
    float carryBonus = 1.0f;      // bônus de transporte
};

// recursos da árvore
struct TreeResources {
    float waterLevel = 100.0f;
    float lightLevel = 100.0f;
    float mineralLevel = 100.0f;
    float treeHealth = 100.0f;
    float maxLevel = 100.0f;
    float consumptionRate = 1.0f; // taxa de consumo por frame
    std::vector<Puddle> waterPuddles;
    std::vector<Puddle> minerals;
    float rainTimer = 10.0f;
    bool isRaining = false;
    float rainDuration = 0.0f;
    float gameTime = 0.0f;
};

// estado de cada folha
struct LeafStatus {
    int nodeIndex;       // índice do nó da folha
    bool active = true;  // está ativa para coleta
    float readyTimer = 0.0f; // tempo até próxima coleta
    float maxCooldown = 7.0f;
};

// galhos caindo
struct FallingBranch {
    Vector2 p1, p2;       // pontos do galho
    Vector2 velocity;      // velocidade do movimento
    float alpha;           // transparência
    float lifeTimer;       // tempo de vida restante
    float thickness;       // espessura do galho
};

// registries globais
inline std::vector<LeafStatus> leafRegistry;         // controle de folhas
inline std::vector<Connection> virtualConnections;  // conexões virtuais
inline std::vector<FallingBranch> fallingBranches;  // galhos caindo
inline int gSugarCount = 0;                          // contador de açúcar
inline int gSugarOrders = 0;                         // ordens de açúcar
inline float gBudCount = 1.0f;                       // contador de brotos
inline CollectionFocus gCurrentFocus = FOCUS_NONE;  // foco de coleta atual

// inicializa o registro de folhas a partir dos nós
inline void InitLeafRegistry(const std::vector<Node>& nodes){
    leafRegistry.clear();
    for(int i = 0; i < (int)nodes.size(); i++){
        if(nodes[i].type == LEAF) leafRegistry.push_back({i, true, 0.0f});
    }
}

#endif
