#include "raylib.h"
#include "Engine.h"
#include <iostream>
#include "rlgl.h"

int main() {

    const int screenWidth = 800;
    const int screenHeight = 600;

    // Create the window and set FPS
    InitWindow(screenWidth, screenHeight, "StarLight Engine");
    SetTargetFPS(60);

    rlDisableBackfaceCulling();

    Engine engine;

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
       

        EndDrawing();
    }

    CloseWindow(); 

    return 0;
}