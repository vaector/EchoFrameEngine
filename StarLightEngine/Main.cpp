#include "raylib.h"
#include "Engine.h"
#include <iostream>
#include "rlgl.h"
#include "Importer.h"
#include "imgui/rlImGui.h"
#include "ImguiWindow.h"

int main() {
    
    const int screenWidth = 1680;
    const int screenHeight = 1024;

    // Create the window and set FPS
    
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "StarLight Engine");
    //ToggleBorderlessWindowed();
    SetTargetFPS(60);

    //ImGui Context
    rlImGuiSetup(true);
    //Initialize Audio
	InitAudioDevice();

    //Init Textures(Images)
    Importer::InitTextures();
    Importer::InitModels();
    Importer::InitModelAnimationNames();

    //Init Sounds and Music
    Importer::InitSounds();
    Importer::InitMusic();

    Engine engine;

	//Reserve memory for sectors and walls to improve performance
    Shared::sectors.reserve(1024);
    Shared::walls.reserve(1024);

    ImguiWindow imgui;
    

    bool freeCamMode = true;

    while (!WindowShouldClose())
    {
    
		//Press F to switch between Editor Mode and Real Mode
        if (IsKeyPressed(KEY_F) && Shared::editorMode) {
            Shared::editorMode = false;//RealEditor
            Shared::setPlayerPos = true;
            DisableCursor();
        }
        else if (IsKeyPressed(KEY_F) && !Shared::editorMode) {
            Shared::editorMode = true;
            EnableCursor();
        }

        //Press P to switch from freecam to playerCam in real Editor Mode
        if(IsKeyPressed(KEY_P) && !Shared::editorMode){
            freeCamMode = !freeCamMode;
            DisableCursor();
        }


        // Update
        if (Shared::editorMode)//TileEditor Updates
            engine.UpdateGui();
        else if (!Shared::editorMode) {//Real Editor Updates
            if(freeCamMode)
                engine.UpdateReal(&engine.realEditor.freeCam);
            else {
                engine.UpdateReal(&engine.realEditor.playerCam);
                
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(BLACK); 


        if (Shared::editorMode) {
            
            //GUI(2d Stuff)
            BeginMode2D(engine.tileEditor.cam);

            engine.DrawGui();

            EndMode2D();
        }
        else if (!Shared::editorMode) {
            //Real(3d Stuff)
            if(freeCamMode)
                BeginMode3D(engine.realEditor.freeCam);
            else
                BeginMode3D(engine.realEditor.playerCam);

            engine.DrawReal();

            EndMode3D();

            //GUI FOR 3D
                //Crosshair
            DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 1, WHITE);
        }

        //Imgui
        imgui.UpdateImguiTileEditor();

        EndDrawing();
    }

    //Unload
    Importer::UnloadTextures();
    Importer::UnloadModels();
    Importer::UnloadSounds();
    Importer::UnloadMusic();

    rlImGuiShutdown();
	CloseAudioDevice();
    CloseWindow(); 

    return 0;
}