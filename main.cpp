#include "raylib.h"
#include "raymath.h"
#include "types.h"
#include "tree_manager.h"
#include "environment.h"
#include "worm_manager.h"
#include "game_init.h"
#include "game_input.h"
#include "game_render.h"
#include <vector>

int main(){
    // inicializa janela em fullscreen
    InitWindow(0, 0, "Eco-Logistics: Survival");
    if (!IsWindowFullscreen()) ToggleFullscreen();
    
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // configura parâmetros do mundo
    WorldConfig config = { (float)sw, (float)sh, sh * 0.65f, sw / 2.0f };

    // estruturas principais do jogo
    TreeResources treeRes;
    std::vector<Node> nodes;
    std::vector<Connection> connections;
    std::vector<Unit> units;
    
    // inicializa mundo e árvore
    SetupNewGame(treeRes, nodes, connections, config);
    InitLeafRegistry(nodes);

    // cria a unidade inicial
    units.push_back({{config.centerX, config.groundLevel}, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});

    // registra unidades para o sistema de input
    RegisterUnits(units);

    // controle da intro
    float introTimer = 0.0f;
    float introDuration = 4.0f;
    bool introFinished = false;

    // estado do jogo
    bool collapsed = false;
    bool paused = false;

    // câmera inicial (zoom alto no começo)
    Camera2D camera = {
        { (float)sw/2, (float)sh/2 },
        { config.centerX, config.groundLevel },
        0,
        3.0f
    };

    SetTargetFPS(60);

    // loop principal
    while (!WindowShouldClose()){

        // pausa com espaço
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        // input de construção/reset
        HandleConstruction(nodes, connections, treeRes, camera,
                           config.groundLevel, config.centerX,
                           introTimer, introFinished);

        // atualiza intro (zoom + tempo)
        if (!introFinished){
            introTimer += GetFrameTime();

            // zoom suavemente de perto → normal
            camera.zoom = Lerp(3.0f, 1.0f, fminf(introTimer / introDuration, 1.0f));

            if (introTimer >= introDuration){
                introFinished = true;
                camera.zoom = 1.0f;
            }
        }

        // gameplay normal
        if (!collapsed && introFinished){

            // controle da câmera
            HandleCamera(camera, config.centerX, config.groundLevel);

            if (!paused){

                // garante pelo menos uma unidade ativa
                if (nodes.size() >= 2 && units.empty()) {
                    units.push_back({{config.centerX, config.groundLevel}, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});
                }

                // atualiza toda simulação (IA + recursos + etc)
                UpdateEcosystem(units, nodes, connections, treeRes, sw, sh, camera);
            }

            // verifica colapso da árvore
            if (treeRes.treeHealth <= 0)
                collapsed = true;
        }

        // evita travar em estado de colapso se recuperar
        if (!nodes.empty() && treeRes.treeHealth > 0)
            collapsed = false;

        // -----------------------------
        // RENDER
        // -----------------------------
        BeginDrawing();
            ClearBackground(BLACK);

            if (!collapsed){

                BeginMode2D(camera);

                    // desenha intro ou jogo normal
                    if (!introFinished){
                        DrawIntro(introTimer, introDuration,
                                  config.centerX, config.groundLevel,
                                  nodes);
                    } else {
                        DrawEcosystem(units, nodes, connections,
                                      treeRes, config.groundLevel,
                                      sw, sh, camera, paused);
                    }

                EndMode2D();
                
                // UI só aparece depois da intro
                if (introFinished){
                    DrawUI(treeRes, sw, sh, nodes,
                           config.groundLevel, camera);
                }

                // overlays (fade / pause)
                DrawGameOverlay(introFinished, introTimer, paused, sw, sh);

            } else {
                // tela de game over
                DrawGameOver(sw, sh);
            }

        EndDrawing();
    }

    // encerra aplicação
    CloseWindow();
    return 0;
}
