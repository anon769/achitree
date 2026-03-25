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

int main() {
    InitWindow(0, 0, "Eco-Logistics: Survival");
    if (!IsWindowFullscreen()) ToggleFullscreen();
    
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    WorldConfig config = { (float)sw, (float)sh, sh * 0.65f, sw / 2.0f };

    TreeResources treeRes;
    std::vector<Node> nodes;
    std::vector<Connection> connections;
    std::vector<Unit> units;
    
    SetupNewGame(treeRes, nodes, connections, config);
    InitLeafRegistry(nodes);
    units.push_back({{config.centerX, config.groundLevel}, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});

    RegisterUnits(units);

    float introTimer = 0.0f;
    float introDuration = 4.0f;
    bool introFinished = false;
    bool collapsed = false;
    bool paused = false;

    Camera2D camera = { { (float)sw/2, (float)sh/2 }, { config.centerX, config.groundLevel }, 0, 3.0f };

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        HandleConstruction(nodes, connections, treeRes, camera, config.groundLevel, config.centerX, introTimer, introFinished);

        if (!introFinished) {
            introTimer += GetFrameTime();
            camera.zoom = Lerp(3.0f, 1.0f, fminf(introTimer / introDuration, 1.0f));
            if (introTimer >= introDuration) {
                introFinished = true;
                camera.zoom = 1.0f;
            }
        }

        if (!collapsed && introFinished) {
            HandleCamera(camera, config.centerX, config.groundLevel);
            if (!paused) {
                if (nodes.size() >= 2 && units.empty()) {
                    units.push_back({{config.centerX, config.groundLevel}, 0, 1, 0.0f, UNIT_SPEED_NORMAL, NONE});
                }
                UpdateEcosystem(units, nodes, connections, treeRes, sw, sh, camera);
            }
            if (treeRes.treeHealth <= 0) collapsed = true;
        }

        if (!nodes.empty() && treeRes.treeHealth > 0) collapsed = false;

        BeginDrawing();
            ClearBackground(BLACK);
            if (!collapsed) {
                BeginMode2D(camera);
                    if (!introFinished) {
                        DrawIntro(introTimer, introDuration, config.centerX, config.groundLevel, nodes);
                    } else {
                        DrawEcosystem(units, nodes, connections, treeRes, config.groundLevel, sw, sh, camera, paused);
                    }
                EndMode2D();
                
                if (introFinished) {
                    DrawUI(treeRes, sw, sh, nodes, config.groundLevel, camera);
                }
                DrawGameOverlay(introFinished, introTimer, paused, sw, sh);
            } else {
                DrawGameOver(sw, sh);
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
