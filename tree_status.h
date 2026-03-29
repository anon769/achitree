#ifndef TREE_STATUS_H
#define TREE_STATUS_H

#include "types.h"

// atualiza níveis básicos da árvore (consumo passivo)
inline void UpdateTreeHealth(TreeResources& res, float deltaTime){
    // consumo contínuo de água e luz
    res.waterLevel = fmaxf(res.waterLevel - res.consumptionRate * deltaTime, 0.0f);
    res.lightLevel = fmaxf(res.lightLevel - res.consumptionRate * deltaTime, 0.0f);

    // garante que não ultrapassem o máximo
    if (res.waterLevel > res.maxLevel)
        res.waterLevel = res.maxLevel;

    if (res.lightLevel > res.maxLevel)
        res.lightLevel = res.maxLevel;
}

#endif
