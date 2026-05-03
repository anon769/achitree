#ifndef MENU_H
#define MENU_H

#include "types.h"
#include "raylib.h"

// função do menu
void ShowMenu() {
    // definição dos botões
    Rectangle btnStart = { (float)GetScreenWidth()/2 - 100, 400, 200, 60 };
    Rectangle btnConfig = { (float)GetScreenWidth()/2 - 100, 500, 200, 60 };
    Rectangle btnExit  = { (float)GetScreenWidth()/2 - 100, 600, 200, 60 };

    // verificar se o mouse está em cima dos botões
    bool mouseOnStart = CheckCollisionPointRec(GetMousePosition(), btnStart);
    bool mouseOnExit  = CheckCollisionPointRec(GetMousePosition(), btnExit);
    bool mouseOnConfig = CheckCollisionPointRec(GetMousePosition(), btnConfig);

    // lógica de Clique
    if (mouseOnStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gCurrentState = GameState::STATE_PLAYING;
    }
    if (mouseOnConfig && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        gCurrentState = GameState::STATE_PLAYING;
    }
    if (mouseOnExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gCurrentState = GameState::STATE_EXIT;
    }

    // visual do menu
    BeginDrawing();
        ClearBackground(BLACK); // Cor de fundo do menu

        // botão jogar
        DrawRectangleRec(btnStart, mouseOnStart ? GetColor(0x36B53CFF) : GetColor(0x18961CFF));
        DrawText("Jogar", btnStart.x + 60, btnStart.y + 20, 20, WHITE);

        // botão configurações
        DrawRectangleRec(btnConfig, mouseOnConfig ? BLUE : DARKBLUE);
        DrawText("Configurações", btnConfig.x + 30, btnConfig.y + 20, 20, WHITE);

        // botão sair
        DrawRectangleRec(btnExit, mouseOnExit ? GetColor(0xC92A2AFF) : GetColor(0x910A0AFF));
        DrawText("Sair", btnExit.x + 70, btnExit.y + 20, 20, WHITE);

    EndDrawing();
}

#endif