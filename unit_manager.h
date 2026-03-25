#ifndef UNIT_MANAGER_H
#define UNIT_MANAGER_H

#include "types.h"

#define UNIT_SPEED_NORMAL 180.0f

void AddUnit(std::vector<Unit>& units, Vector2 startPos) {
    units.push_back({startPos, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});
}

void RemoveUnit(std::vector<Unit>& units) {
    if (!units.empty()) units.pop_back();
}

void HandleUnitInput(std::vector<Unit>& units, Vector2 spawnPos, TreeResources& res) {
    if (IsKeyPressed(KEY_UP)) {
        AddUnit(units, spawnPos);
    }
    if (IsKeyPressed(KEY_DOWN)) {
        RemoveUnit(units);
    }
    if (IsKeyPressed(KEY_S)) {
        gSugarOrders++;
    }
    if (IsKeyPressed(KEY_U)) {
        if (gSugarCount >= 1) {
            gSugarCount--;
            AddUnit(units, spawnPos);
        }
    }
    if (IsKeyPressed(KEY_B)) {
        if (gSugarCount >= 1 && res.mineralLevel >= 20.0f) {
            gSugarCount--;
            res.mineralLevel -= 20.0f;
            gBudCount++;
        }
    }
    if (IsKeyPressed(KEY_W)) gCurrentFocus = FOCUS_WATER;
    if (IsKeyPressed(KEY_L)) gCurrentFocus = FOCUS_LIGHT;
    if (IsKeyPressed(KEY_M)) gCurrentFocus = FOCUS_MINERAL;
    if (IsKeyPressed(KEY_D)) gCurrentFocus = FOCUS_NONE;
}

#endif
