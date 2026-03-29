#ifndef UNIT_MANAGER_H
#define UNIT_MANAGER_H

#include "types.h"

// velocidade padrão das unidades
#define UNIT_SPEED_NORMAL 180.0f

// adiciona uma nova unidade na posição inicial
void AddUnit(std::vector<Unit>& units, Vector2 startPos){
    units.push_back({startPos, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});
}

// remove a última unidade da lista
void RemoveUnit(std::vector<Unit>& units){
    if (!units.empty()) units.pop_back();
}

// processa entradas do usuário relacionadas às unidades e recursos
void HandleUnitInput(std::vector<Unit>& units, Vector2 spawnPos, TreeResources& res){
    // adiciona ou remove unidades com UP/DOWN (!TODO apenas para teste)
    if (IsKeyPressed(KEY_UP)) AddUnit(units, spawnPos);
    if (IsKeyPressed(KEY_DOWN)) RemoveUnit(units);

    // incrementa/decrementa ordens de açúcar
    if (IsKeyPressed(KEY_S)){
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)){
            if (gSugarOrders > 0){
                gSugarOrders--; // Shift + S
            }
        } else {
            gSugarOrders++; // S normal
        }
    }

    // cria unidade consumindo 1 açúcar
    if (IsKeyPressed(KEY_U)){
        if (gSugarCount >= 1){
            gSugarCount--;
            AddUnit(units, spawnPos);
        }
    }

    // cria broto consumindo açúcar e minerais
    if (IsKeyPressed(KEY_B)){
        if (gSugarCount >= 1 && res.mineralLevel >= 20.0f){
            gSugarCount--;
            res.mineralLevel -= 20.0f;
            gBudCount++;
        }
    }

    // define foco de coleta
    if (IsKeyPressed(KEY_W)) gCurrentFocus = FOCUS_WATER;
    if (IsKeyPressed(KEY_L)) gCurrentFocus = FOCUS_LIGHT;
    if (IsKeyPressed(KEY_M)) gCurrentFocus = FOCUS_MINERAL;
    if (IsKeyPressed(KEY_D)) gCurrentFocus = FOCUS_NONE;
}

#endif
