#include "raylib.h"
#include "Engine.h"
#include <iostream>
#include "rlgl.h"
#include "Importer.h"
#include "imgui/rlImGui.h"
#include "ImguiWindow.h"

int main() {

    const int screenWidth = 1024;
    const int screenHeight = 1024;

    // Create the window and set FPS
    InitWindow(screenWidth, screenHeight, "StarLight Engine");
    SetTargetFPS(60);

    //ImGui Context
    rlImGuiSetup(true);

    //Init Textures(Images)
    Importer::InitTextures();

    Engine engine;

    ImguiWindow imgui;

    bool editorMode = true;

    while (!WindowShouldClose())
    {

        if (IsKeyPressed(KEY_F) && editorMode) {
            editorMode = false;
            DisableCursor();
        }
        else if (IsKeyPressed(KEY_F) && !editorMode) {
            editorMode = true;
            EnableCursor();
        }

        // Update
        if (editorMode)
            engine.UpdateGui();
        else if(!editorMode)
            engine.UpdateReal();

        // Draw
        BeginDrawing();
        ClearBackground(BLACK); 


        if (editorMode) {
            
            //GUI(2d Stuff)
            BeginMode2D(engine.tileEditor.cam);

            engine.DrawGui();

            EndMode2D();
        }
        else if (!editorMode) {
            //Real(3d Stuff)
            BeginMode3D(engine.realEditor.cam);

            engine.DrawReal();

            EndMode3D();
        }

        //Imgui
        imgui.UpdateImguiTileEditor();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow(); 

    return 0;
}