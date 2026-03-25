#ifndef TYPES_H
#define TYPES_H

#include "raylib.h"
#include <vector>

enum NodeType { TRUNK, BRANCH, ROOT, LEAF };
enum ResourceType { NONE, WATER, LIGHT, MINERAL, SUGAR };
enum CollectionFocus { FOCUS_NONE, FOCUS_WATER, FOCUS_LIGHT, FOCUS_MINERAL };

struct Node {
    Vector2 position;
    NodeType type;
};

struct Connection {
    int from;
    int to;
};

struct Puddle {
    Vector2 position;
    float width;
    float height;
    float amount;
};

struct Unit {
    Vector2 currentPos;
    int startNodeIndex;
    int targetNodeIndex;
    float progress;
    float speed;
    ResourceType carrying = NONE;
    ResourceType intent = NONE;
    float carryBonus = 1.0f;
};

struct TreeResources {
    float waterLevel = 100.0f;
    float lightLevel = 100.0f;
    float mineralLevel = 100.0f;
    float treeHealth = 100.0f;
    float maxLevel = 100.0f;
    float consumptionRate = 1.8f;
    std::vector<Puddle> waterPuddles;
    std::vector<Puddle> minerals;
    float rainTimer = 10.0f;
    bool isRaining = false;
    float rainDuration = 0.0f;
    float gameTime = 0.0f;
};

struct LeafStatus {
    int nodeIndex;
    bool active = true;
    float readyTimer = 0.0f;
    float maxCooldown = 7.0f;
};

struct FallingBranch {
    Vector2 p1, p2;
    Vector2 velocity;
    float alpha;
    float lifeTimer;
    float thickness;
};

inline std::vector<LeafStatus> leafRegistry;
inline std::vector<Connection> virtualConnections;
inline std::vector<FallingBranch> fallingBranches;
inline int gSugarCount = 0;
inline int gSugarOrders = 0;
inline float gBudCount = 1.0f;
inline CollectionFocus gCurrentFocus = FOCUS_NONE;

inline void InitLeafRegistry(const std::vector<Node>& nodes) {
    leafRegistry.clear();
    for(int i = 0; i < (int)nodes.size(); i++) {
        if(nodes[i].type == LEAF) leafRegistry.push_back({i, true, 0.0f});
    }
}

#endif
