#include "raylib.h"
#include "Engine.h"

int main() {

    const int screenWidth = 800;
    const int screenHeight = 600;

    // Create the window and set FPS
    InitWindow(screenWidth, screenHeight, "StarLight Engine");
    SetTargetFPS(60);

    Engine engine;

    while (!WindowShouldClose())
    {
        // Update
        engine.UpdateGui();

        // Draw
        BeginDrawing();
        ClearBackground(BLACK); 



            //GUI(2d Stuff)
        BeginMode2D(engine.tileEditor.cam);
   
        engine.DrawGui();
        EndMode2D();

        EndDrawing();
    }

    CloseWindow(); 

    return 0;
}