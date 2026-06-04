#ifndef MENU_H
#define MENU_H

#include "types.h"
#include "raylib.h"

// visual do menu principal
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
        gCurrentState = GameState::STATE_SETTINGS;
    }
    if (mouseOnExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        gCurrentState = GameState::STATE_EXIT;
    }

    // visual do menu
    ClearBackground(BLACK); // Cor de fundo do menu

    // botão jogar
    DrawRectangleRec(btnStart, mouseOnStart ? GetColor(0x36B53CFF) : GetColor(0x18961CFF));
    DrawText("Jogar", btnStart.x + 70, btnStart.y + 20, 20, WHITE);

    // botão configurações
    DrawRectangleRec(btnConfig, mouseOnConfig ? BLUE : DARKBLUE);
    DrawText("Configurações", btnConfig.x + 30, btnConfig.y + 20, 20, WHITE);

    // botão sair
    DrawRectangleRec(btnExit, mouseOnExit ? GetColor(0xC92A2AFF) : GetColor(0x910A0AFF));
    DrawText("Sair", btnExit.x + 80, btnExit.y + 20, 20, WHITE);
}

// visual do menu de configurações
void ShowSettings() {
    // definição dos botões da tela de configurações
    Rectangle btnAudio  = { (float)GetScreenWidth()/2 - 100, 300, 200, 60 };
    Rectangle btnKeys   = { (float)GetScreenWidth()/2 - 100, 400, 200, 60 };
    Rectangle btnVoltar = { (float)GetScreenWidth()/2 - 100, 550, 200, 60 };

    // verificar colisão do mouse
    bool mouseOnAudio  = CheckCollisionPointRec(GetMousePosition(), btnAudio);
    bool mouseOnKeys   = CheckCollisionPointRec(GetMousePosition(), btnKeys);
    bool mouseOnVoltar = CheckCollisionPointRec(GetMousePosition(), btnVoltar);

    // opções de configuração
    if (mouseOnAudio && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // config de áudio ...
    }
    if (mouseOnKeys && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // config de trocar teclas ...
    }
    if (mouseOnVoltar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // sair do menu
        gCurrentState = GameState::STATE_MENU; 
    }

    // visual das configurações
    ClearBackground(BLACK);

    // título da aba
    DrawText("CONFIGURAÇÕES", (float)GetScreenWidth()/2 - 100, 150, 30, WHITE);

    // botão áudio
    DrawRectangleRec(btnAudio, mouseOnAudio ? BLUE : DARKBLUE);
    DrawText("Áudio", btnAudio.x + 70, btnAudio.y + 20, 20, WHITE);

    // botão teclas
    DrawRectangleRec(btnKeys, mouseOnKeys ? BLUE : DARKBLUE);
    DrawText("Teclas", btnKeys.x + 70, btnKeys.y + 20, 20, WHITE);

    // botão voltar ao menu principal
    DrawRectangleRec(btnVoltar, mouseOnVoltar ? GetColor(0xC92A2AFF) : GetColor(0x910A0AFF));
    DrawText("Voltar ao menu", btnVoltar.x + 65, btnVoltar.y + 20, 20, WHITE);
}

// visual do menu de pausa
void ShowPause() {
    // definição dos botões da tela de configurações
    Rectangle btnContinue = { (float)GetScreenWidth()/2 - 100, 300, 200, 60 };
    Rectangle btnSettings   = { (float)GetScreenWidth()/2 - 100, 400, 200, 60 };
    Rectangle btnVoltar = { (float)GetScreenWidth()/2 - 100, 550, 200, 60 };

    // verificar colisão do mouse
    bool mouseOnContinue  = CheckCollisionPointRec(GetMousePosition(), btnContinue);
    bool mouseOnSettings   = CheckCollisionPointRec(GetMousePosition(), btnSettings);
    bool mouseOnVoltar = CheckCollisionPointRec(GetMousePosition(), btnVoltar);

    // opções do menu de pausa
    if (mouseOnContinue && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // sair da pausa
        gCurrentState = GameState::STATE_PLAYING; 
    }
    if (mouseOnSettings && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // abrir configurações
        gCurrentState = GameState::STATE_SETTINGS; 
    }
    if (mouseOnVoltar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // sair do menu
        gCurrentState = GameState::STATE_MENU; 
    }

    // visual das configurações
    ClearBackground(BLACK);

    // título da aba
    DrawText("PAUSE", (float)GetScreenWidth()/2 - 100, 150, 30, WHITE);

    // botão continuar
    DrawRectangleRec(btnContinue, mouseOnContinue ? BLUE : DARKBLUE);
    DrawText("Continuar", btnContinue.x + 70, btnContinue.y + 20, 20, WHITE);

    // botão teclas
    DrawRectangleRec(btnSettings, mouseOnSettings ? BLUE : DARKBLUE);
    DrawText("Teclas", btnSettings.x + 70, btnSettings.y + 20, 20, WHITE);

    // botão voltar ao menu principal
    DrawRectangleRec(btnVoltar, mouseOnVoltar ? GetColor(0xC92A2AFF) : GetColor(0x910A0AFF));
    DrawText("Voltar ao menu", btnVoltar.x + 65, btnVoltar.y + 20, 20, WHITE);
}

#endif