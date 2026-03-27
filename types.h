#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <vector>

// Tipos de nó na árvore
enum NodeType { TRUNK, BRANCH, ROOT, LEAF };

// Tipos de recurso que unidades podem carregar
enum ResourceType { NONE, WATER, LIGHT, MINERAL, SUGAR };

// Foco de coleta de recursos
enum CollectionFocus { FOCUS_NONE, FOCUS_WATER, FOCUS_LIGHT, FOCUS_MINERAL };

// Nó da árvore
struct Node {
    Vector2 position; // Posição na tela/mundo
    NodeType type;    // Tipo do nó
};

// Conexão entre dois nós
struct Connection {
    int from;
    int to;
};

// Representa poças de água ou minerais
struct Puddle {
    Vector2 position;
    float width;
    float height;
    float amount; // Quantidade de recurso
};

// Unidade que se move e coleta recursos
struct Unit {
    Vector2 currentPos;      // Posição atual
    int startNodeIndex;      // Nó inicial
    int targetNodeIndex;     // Nó alvo
    float progress;          // Progresso do movimento
    float speed;             // Velocidade
    ResourceType carrying = NONE; // Recurso carregado
    ResourceType intent = NONE;   // Intenção de coleta
    float carryBonus = 1.0f;      // Bônus de transporte
};

// Recursos da árvore
struct TreeResources {
    float waterLevel = 100.0f;
    float lightLevel = 100.0f;
    float mineralLevel = 100.0f;
    float treeHealth = 100.0f;
    float maxLevel = 100.0f;
    float consumptionRate = 1.8f; // Taxa de consumo por frame
    std::vector<Puddle> waterPuddles;
    std::vector<Puddle> minerals;
    float rainTimer = 10.0f;
    bool isRaining = false;
    float rainDuration = 0.0f;
    float gameTime = 0.0f;
};

// Estado de cada folha
struct LeafStatus {
    int nodeIndex;       // Índice do nó da folha
    bool active = true;  // Está ativa para coleta
    float readyTimer = 0.0f; // Tempo até próxima coleta
    float maxCooldown = 7.0f;
};

// Galhos caindo
struct FallingBranch {
    Vector2 p1, p2;       // Pontos do galho
    Vector2 velocity;      // Velocidade do movimento
    float alpha;           // Transparência
    float lifeTimer;       // Tempo de vida restante
    float thickness;       // Espessura do galho
};

// Registries globais
inline std::vector<LeafStatus> leafRegistry;         // Controle de folhas
inline std::vector<Connection> virtualConnections;  // Conexões virtuais
inline std::vector<FallingBranch> fallingBranches;  // Galhos caindo
inline int gSugarCount = 0;                          // Contador de açúcar
inline int gSugarOrders = 0;                         // Ordens de açúcar
inline float gBudCount = 1.0f;                       // Contador de brotos
inline CollectionFocus gCurrentFocus = FOCUS_NONE;  // Foco de coleta atual

// Inicializa o registro de folhas a partir dos nós
inline void InitLeafRegistry(const std::vector<Node>& nodes) {
    leafRegistry.clear();
    for(int i = 0; i < (int)nodes.size(); i++) {
        if(nodes[i].type == LEAF) leafRegistry.push_back({i, true, 0.0f});
    }
}

#endif
