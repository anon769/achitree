#ifndef TREE_STATUS_H
#define TREE_STATUS_H
#include "types.h"
inline void UpdateTreeHealth(TreeResources& res, float deltaTime) {
    res.waterLevel = fmaxf(res.waterLevel - res.consumptionRate * deltaTime, 0.0f);
    res.lightLevel = fmaxf(res.lightLevel - res.consumptionRate * deltaTime, 0.0f);
    if (res.waterLevel > res.maxLevel) res.waterLevel = res.maxLevel;
    if (res.lightLevel > res.maxLevel) res.lightLevel = res.maxLevel;
}
#endif
