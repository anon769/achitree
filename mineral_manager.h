#ifndef MINERAL_MANAGER_H
#define MINERAL_MANAGER_H

#include "types.h"
#include "raymath.h"

// atualiza geração de minerais no mapa
inline void UpdateMinerals(TreeResources& res, float groundLevel, float centerX, float dt) {
    
    // chance aleatória de spawn + limite máximo de minerais
    if (GetRandomValue(0, 100) < 5 && res.minerals.size() < 25) {

        // posição horizontal aleatória
        float px = (float)GetRandomValue(centerX - 950, centerX + 950);

        // define profundidade (mais chance de aparecer mais raso)
        float py = (GetRandomValue(1, 3) <= 2) ? 
                   (float)GetRandomValue((int)groundLevel + 50, (int)groundLevel + 450) : 
                   (float)GetRandomValue((int)groundLevel + 451, (int)groundLevel + 950);
        
        // tamanho do depósito mineral
        float pw = (float)GetRandomValue(60, 120);

        bool tooClose = false;

        // evita gerar minerais muito próximos uns dos outros
        for (const auto& m : res.minerals) {
            if (Vector2Distance({px, py}, m.position) < 150.0f) {
                tooClose = true;
                break;
            }
        }

        // adiciona se a posição for válida
        if (!tooClose)
            res.minerals.push_back({{px, py}, pw, pw * 0.6f, 100.0f});
    }
}

#endif
