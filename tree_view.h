#ifndef TREE_VIEW_H
#define TREE_VIEW_H

#include "types.h"
#include "tree_utils.h"
#include "environment.h"
#include "worm_manager.h"

// Desenha o ecossistema: chão, poeira, chuva, galhos, conexões, folhas e unidades
inline void DrawEcosystem(const std::vector<Unit>& units, const std::vector<Node>& nodes, const std::vector<Connection>& connections, const TreeResources& res, float groundLevel, int sw, int sh, Camera2D camera, bool paused) {
    float centerX = sw / 2.0f; 
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);

    // Controle de fade da poeira conforme o vento
    static float dustFade = 0.0f;
    float targetFade = (gWeather.windStrength > 0.42f) ? 1.0f : 0.0f;
    if (!paused) dustFade = Lerp(dustFade, targetFade, GetFrameTime() * 2.0f);

    // Desenha o chão
    DrawRectangle(centerX - 1000, (int)groundLevel, 2000, 1000, {15, 15, 15, 255});
    DrawWormTrails(); 
    DrawWorms();

    // Contorno da área
    DrawRectangleLinesEx({ centerX - 1000, groundLevel - 1000, 2000, 2000 }, 5.0f, RED);

    // Desenha poeira se houver vento
    if (dustFade > 0.001f) {
        for (int i = 0; i < 80; i++) {
            float speed = 2000.0f * gWeather.windStrength;
            float time = (paused ? 1.0f : (float)GetTime() * 2.0f) + (i * 0.5f);
            float offset = fmodf(i * 157.0f + (paused ? 0 : (float)GetTime() * speed), 2000.0f);
            float px = (centerX - 1000) + offset;
            float jumpHeight = 15.0f + (i % 20) * 5.0f;
            float arc = fabsf(sinf(time)) * jumpHeight;
            float py = groundLevel - 2 - arc; 
            float pSize = (float)GetRandomValue(2, 6);

            if (arc < 2.0f) {
                DrawRectangleV({ px, groundLevel - pSize }, { pSize * 2.0f, pSize }, Fade({ 80, 50, 25, 230 }, dustFade));
            } else {
                DrawRectangleV({ px, py }, { pSize, pSize }, Fade({ 101, 67, 33, 200 }, dustFade));
            }
        }
    }

    // Desenha chuva
    if (res.isRaining) {
        float rainAngle = gWeather.windStrength * 10.0f;
        for (int i = 0; i < 100; i++) {
            int rx = GetRandomValue(centerX - 1000, centerX + 1000);
            int ry = GetRandomValue(groundLevel - 1000, groundLevel);
            DrawLine(rx, ry, rx - rainAngle, ry + 10, {100, 150, 255, 150});
        }
    }
    
    // Desenha poças de água e minerais
    for (auto& p : res.waterPuddles) DrawEllipse((int)p.position.x, (int)p.position.y, (int)p.width / 2, (int)p.height / 2, {30, 80, 150, (unsigned char)Clamp(p.amount * 2.0f, 0, 255)});
    for (auto& m : res.minerals) DrawEllipse((int)m.position.x, (int)m.position.y, (int)m.width / 2, (int)m.height / 2, {130, 50, 200, (unsigned char)Clamp(m.amount * 2.0f, 0, 255)});

    // Desenha galhos caindo
    for (auto& fb : fallingBranches) {
        DrawLineEx(fb.p1, fb.p2, fb.thickness, Fade(DARKBROWN, fb.alpha));
        Vector2 v1 = { fb.p2.x, fb.p2.y - 12 }; 
        Vector2 v2 = { fb.p2.x - 10, fb.p2.y + 4 }; 
        Vector2 v3 = { fb.p2.x + 10, fb.p2.y + 4 };
        DrawTriangle(v1, v2, v3, Fade(GREEN, fb.alpha));
    }

    // Desenha conexões virtuais (sem relação física)
    for (auto& vc : virtualConnections) {
        Vector2 p1 = Vector2Add(nodes[vc.from].position, GetWindOffset(nodes[vc.from].position, groundLevel, nodes[vc.from].type, nodes));
        Vector2 p2 = Vector2Add(nodes[vc.to].position, GetWindOffset(nodes[vc.to].position, groundLevel, nodes[vc.to].type, nodes));
        DrawLineEx(p1, p2, 1.5f, {139, 69, 19, 100});
    }
    
    // Desenha conexões reais da árvore
    for (auto& c : connections) {
        float thickness = (nodes[c.from].type == TRUNK && nodes[c.to].type == TRUNK) ? 8.0f : 3.0f;
        Vector2 p1 = Vector2Add(nodes[c.from].position, GetWindOffset(nodes[c.from].position, groundLevel, nodes[c.from].type, nodes));
        Vector2 p2 = Vector2Add(nodes[c.to].position, GetWindOffset(nodes[c.to].position, groundLevel, nodes[c.to].type, nodes));
        DrawLineEx(p1, p2, thickness, DARKBROWN);
    }

    // Interação com o mouse para crescimento de novos galhos
    if (!paused) {
        bool holdingT = IsKeyDown(KEY_T);
        bool mouseBelow = (mouseWorld.y > groundLevel + 2.0f);
        int closestIdx = -1;
        float minDist = 250.0f;

        for (int i = 0; i < (int)nodes.size(); i++) {
            if (nodes[i].type != TRUNK && nodes[i].type != ROOT) {
                bool connected = false;
                for(auto& c : connections) if(c.from == i || c.to == i) { connected = true; break; }
                if(!connected) continue;
            }
            if (mouseBelow && (nodes[i].type != ROOT && nodes[i].type != TRUNK)) continue;
            if (!mouseBelow && nodes[i].type == ROOT) continue;
            Vector2 nodeVisualPos = Vector2Add(nodes[i].position, GetWindOffset(nodes[i].position, groundLevel, nodes[i].type, nodes));
            float d = Vector2Distance(mouseWorld, nodeVisualPos);
            if (d < minDist) { minDist = d; closestIdx = i; }
        }

        if (closestIdx != -1) {
            float maxAllowedDist = (mouseBelow) ? 150.0f : 45.0f;
            Vector2 targetNodePos = Vector2Add(nodes[closestIdx].position, GetWindOffset(nodes[closestIdx].position, groundLevel, nodes[closestIdx].type, nodes));
            if (Vector2Distance(mouseWorld, targetNodePos) <= maxAllowedDist) {
                float budCost = (holdingT && !mouseBelow) ? 2.0f : 1.0f;
                if (mouseBelow) {
                    for (const auto& tp : gWormTrails) {
                        if (Vector2Distance(mouseWorld, tp.position) < 15.0f) {
                            budCost = 0.5f;
                            break;
                        }
                    }
                }
                Color lineColor = (gBudCount >= budCost) ? WHITE : RED;
                DrawLineEx(targetNodePos, mouseWorld, 2.0f, lineColor);
            }
        }
    }

    // Desenha nós da árvore (tronco, raízes, folhas)
    for (int i = 0; i < (int)nodes.size(); i++) {
        if (nodes[i].type != TRUNK && nodes[i].type != ROOT) {
            bool connected = false;
            for(auto& c : connections) if(c.from == i || c.to == i) { connected = true; break; }
            if(!connected) continue;
        }

        Vector2 nodePos = Vector2Add(nodes[i].position, GetWindOffset(nodes[i].position, groundLevel, nodes[i].type, nodes));
        
        if (nodes[i].type == LEAF) {
            bool found = false; float timer = 0.0f; bool isActive = false;
            for (auto& lr : leafRegistry) { if (lr.nodeIndex == i) { found = true; timer = lr.readyTimer; isActive = lr.active; } }
            if (found && isActive) {
                Vector2 v1 = { nodePos.x, nodePos.y - 12 }; 
                Vector2 v2 = { nodePos.x - 10, nodePos.y + 4 }; 
                Vector2 v3 = { nodePos.x + 10, nodePos.y + 4 };
                DrawTriangle(v1, v2, v3, timer > 0 ? DARKGREEN : GREEN);
            }
        } else {
            Color nodeColor = DARKBROWN;
            for (auto& p : res.waterPuddles) if (IsPointInPuddle(nodes[i].position, p)) { nodeColor = BLUE; break; }
            for (auto& m : res.minerals) if (IsPointInPuddle(nodes[i].position, m)) { nodeColor = PURPLE; break; }
            DrawCircleV(nodePos, (nodes[i].type == TRUNK ? 8.0f : 4.0f), nodeColor);
        }
    }
    
    // Desenha unidades e seus recursos carregados
    for (auto& u : units) {
        DrawCircleV(u.currentPos, 5, WHITE);
        if (u.carrying != NONE) {
            Color c = WHITE;
            if (u.carrying == WATER) c = BLUE;
            else if (u.carrying == LIGHT) c = YELLOW;
            else if (u.carrying == MINERAL) c = PURPLE;
            else if (u.carrying == SUGAR) c = BROWN;
            
            if (u.carrying == SUGAR) {
                DrawRectangle((int)u.currentPos.x - 3, (int)u.currentPos.y - 11, 6, 6, c);
            } else {
                DrawCircle((int)u.currentPos.x, (int)u.currentPos.y - 8, 3, c);
            }
        }
    }
}

// Desenha interface de usuário (UI) com barras e informações
inline void DrawUI(const TreeResources& res, int sw, int sh, const std::vector<Node>& nodes, float groundLevel, Camera2D camera) {
    auto Bar = [&](int y, float val, Color c, float previewCost) {
        DrawRectangle(20, y, 180, 12, {40, 40, 40, 255}); // fundo
        int currentW = (int)(180 * (fminf(val, res.maxLevel) / 100.0f));
        DrawRectangle(20, y, currentW, 12, c); // barra
    };

    Vector2 mWorld = GetScreenToWorld2D(GetMousePosition(), camera);
    bool mouseBelow = (mWorld.y > groundLevel + 2.0f);
    bool holdingT = IsKeyDown(KEY_T);
    float previewWater = 0, previewLight = 0;

    // Detecta nó mais próximo para pré-visualização de custo
    int closest = -1;
    float minDist = 250.0f;
    for (int i = 0; i < (int)nodes.size(); i++) {
        bool canConnect = mouseBelow ? (nodes[i].type == ROOT || nodes[i].type == TRUNK) : (nodes[i].type != ROOT);
        if (canConnect) {
            Vector2 evPos = Vector2Add(nodes[i].position, GetWindOffset(nodes[i].position, groundLevel, nodes[i].type, nodes));
            float d = Vector2Distance(mWorld, evPos);
            if (d < minDist) { minDist = d; closest = i; }
        }
    }

    if (closest != -1) {
        Vector2 evParent = Vector2Add(nodes[closest].position, GetWindOffset(nodes[closest].position, groundLevel, nodes[closest].type, nodes));
        float maxD = (mouseBelow) ? 150.0f : 45.0f;
        if (Vector2Distance(mWorld, evParent) <= maxD) {
            float cost = (holdingT && !mouseBelow) ? 80.0f : 40.0f;
            if (mouseBelow) {
                for (const auto& tp : gWormTrails) if (Vector2Distance(mWorld, tp.position) < 15.0f) { cost *= 0.5f; break; }
                previewWater = cost;
            } else {
                previewLight = cost;
            }
        }
    }

    // Desenha barras de recursos
    Bar(20, res.lightLevel, YELLOW, previewLight);
    Bar(40, res.waterLevel, BLUE, previewWater);
    Bar(60, res.mineralLevel, PURPLE, 0);
    Bar(80, res.treeHealth, LIME, 0);

    // Desenha texto de recursos e foco
    DrawText(TextFormat("SUGAR: %d", gSugarCount), sw - 150, 20, 20, BROWN);
    DrawText(TextFormat("BUDS: %.1f", gBudCount), sw - 150, 45, 20, GREEN);
    
    const char* focusStr = "DEFAULT";
    Color focusClr = WHITE;
    if (gCurrentFocus == FOCUS_WATER) { focusStr = "WATER"; focusClr = BLUE; }
    else if (gCurrentFocus == FOCUS_LIGHT) { focusStr = "LIGHT"; focusClr = YELLOW; }
    else if (gCurrentFocus == FOCUS_MINERAL) { focusStr = "MINERAL"; focusClr = PURPLE; }
    
    const char* focusFull = TextFormat("FOCUS: %s", focusStr);
    int focusWidth = MeasureText(focusFull, 20);
    DrawText(focusFull, (sw / 2) - (focusWidth / 2), 20, 20, focusClr);

    // Desenha ordens de açúcar se existirem
    if (gSugarOrders > 0) DrawText(TextFormat("SUGAR ORDERS: %d", gSugarOrders), sw - 220, 100, 15, ORANGE);
}

#endif
