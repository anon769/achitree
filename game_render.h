#ifndef GAME_RENDER_H
#define GAME_RENDER_H

#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include "environment.h"
#include <vector>
#include "world_config.h"

// desenha a animação inicial de crescimento da árvore
inline void DrawIntro(float introTimer, float introDuration, float centerX, float groundLevel, const std::vector<Node>& nodes){
    // progresso da animação (0 → 1)
    float introProgress = fminf(introTimer / introDuration, 1.0f);

    // fundo (solo)
    DrawRectangle(centerX - MAP_LIMIT, (int)groundLevel, MAP_LIMIT * 2, MAP_LIMIT, {15, 15, 15, 255});
    
    // tronco inicial
    DrawCircleV(nodes[0].position, 8.0f, DARKBROWN);
    
    // crescimento da raiz (interpolando posição)
    Vector2 rootEnd = Vector2Lerp(nodes[0].position, nodes[1].position, introProgress);
    DrawLineEx(nodes[0].position, rootEnd, 3.0f, DARKBROWN);
    DrawCircleV(rootEnd, 4.0f, DARKBROWN);
    
    // crescimento da folha
    Vector2 leafEnd = Vector2Lerp(nodes[0].position, nodes[2].position, introProgress);
    DrawLineEx(nodes[0].position, leafEnd, 3.0f, DARKBROWN);
    
    // desenha folha simples (triângulo)
    Vector2 v1 = { leafEnd.x, leafEnd.y - 12 };
    Vector2 v2 = { leafEnd.x - 10, leafEnd.y + 4 };
    Vector2 v3 = { leafEnd.x + 10, leafEnd.y + 4 };
    DrawTriangle(v1, v2, v3, GREEN);
}

// desenha overlay da tela (pause + fade da intro)
inline void DrawGameOverlay(bool introFinished, float introTimer, bool paused, int sw, int sh){
    // escurece a tela se estiver pausado
    if (paused){
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, 0.3f));
    }
    
    // fade-in inicial (vai sumindo com o tempo)
    if (!introFinished){
        float fadeAlpha = Clamp(1.0f - (introTimer / 1.5f), 0.0f, 1.0f);
        DrawRectangle(0, 0, sw, sh, Fade(BLACK, fadeAlpha));
    }
}

// tela de game over
inline void DrawGameOver(int sw, int sh){
    const char* mainText = "The ecosystem has collapsed.";
    const char* subText = "Press ESC to quit or R to restart.";

    int fontSizeMain = 30;
    int fontSizeSub = 20;

    // texto principal centralizado
    DrawText(mainText,
             sw/2 - MeasureText(mainText, fontSizeMain)/2,
             sh/2 - 15,
             fontSizeMain,
             RED);

    // texto secundário
    DrawText(subText,
             sw/2 - MeasureText(subText, fontSizeSub)/2,
             sh/2 + 25,
             fontSizeSub,
             WHITE);
}

#endif
