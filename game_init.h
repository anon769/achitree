#ifndef GAME_INIT_H
#define GAME_INIT_H

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include <vector>

// configuração básica do mundo
struct WorldConfig {
    float sw;          // largura da tela/mundo
    float sh;          // altura da tela/mundo
    float groundLevel; // nível do chão (referência vertical)
    float centerX;     // centro horizontal (onde nasce a árvore)
};

// gera poças de água espalhadas pelo mapa
inline void GenerateWaterPuddles(TreeResources& res, const WorldConfig& config) {
    
    // poça inicial fixa (garante recurso perto do início)
    res.waterPuddles.push_back({{config.centerX, config.groundLevel + 100}, 120, 60, 100.0f});

    // gera poças adicionais aleatórias
    for(int i = 0; i < 5; i++) {
        float px, py;
        bool validPos = false;
        int attempts = 0;

        // tenta achar uma posição válida (evitando sobreposição)
        while (!validPos && attempts < 10) {

            // posição horizontal aleatória ao redor do centro
            px = (float)GetRandomValue(config.centerX - 950, config.centerX + 950);
            
            // define altura com leve bias para mais perto do chão
            if (GetRandomValue(1, 3) <= 2) {
                py = (float)GetRandomValue(config.groundLevel + 50, config.groundLevel + 450);
            } else {
                py = (float)GetRandomValue(config.groundLevel + 451, config.groundLevel + 950);
            }

            validPos = true;

            // evita poças muito próximas umas das outras
            for (const auto& p : res.waterPuddles) {
                if (Vector2Distance({px, py}, p.position) < 200.0f) {
                    validPos = false;
                    break;
                }
            }

            attempts++;
        }

        // tamanho aleatório da poça
        float pw = (float)GetRandomValue(80, 180);

        // adiciona a nova poça
        res.waterPuddles.push_back({{px, py}, pw, pw * 0.5f, 100.0f});
    }
}

// cria a estrutura inicial da árvore (mínima)
inline void InitTreeStructure(std::vector<Node>& nodes, std::vector<Connection>& connections, const WorldConfig& config) {
    
    // nó principal (tronco)
    nodes.push_back({{config.centerX, config.groundLevel}, TRUNK});

    // raiz (abaixo)
    nodes.push_back({{config.centerX, config.groundLevel + 50}, ROOT});

    // folha inicial (acima)
    nodes.push_back({{config.centerX, config.groundLevel - 40}, LEAF});

    // conexões básicas (estrutura inicial)
    connections.push_back({0, 1}); // tronco ↔ raiz
    connections.push_back({0, 2}); // tronco ↔ folha
}

// inicializa um novo jogo
inline void SetupNewGame(TreeResources& res, std::vector<Node>& nodes, std::vector<Connection>& connections, const WorldConfig& config) {
    
    // gera recursos no mapa
    GenerateWaterPuddles(res, config);

    // cria a árvore inicial
    InitTreeStructure(nodes, connections, config);
}

#endif
