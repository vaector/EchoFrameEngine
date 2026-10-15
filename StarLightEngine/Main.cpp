#include "raylib.h"
#include "Engine.h"
#include <iostream>
#include "rlgl.h"
#include "Importer.h"
#include "imgui/rlImGui.h"
#include "ImguiWindow.h"
#include "LightSystem.h"
#include "imgui/imgui.h"

int main() {
    
    const int screenWidth = 1680;
    const int screenHeight = 1024;

    // Create the window and set FPS
    
    SetConfigFlags(FLAG_WINDOW_MAXIMIZED);
    InitWindow(screenWidth, screenHeight, "StarLight Engine");
    ToggleBorderlessWindowed();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

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

    //Init Light
    Importer::InitShaders();
    LightSystem::InitLights();

    Engine engine;

	//Reserve memory for sectors and walls to improve performance
    Shared::sectors.reserve(1024);
    Shared::walls.reserve(1024);

    ImguiWindow imgui;
    

    bool freeCamMode = true;

    bool running = true;
    while (running)
    {
        //Close window upon pressing the Window "Red X" Button.
        if (WindowShouldClose())
            running = false;

		//Press F to switch between Editor Mode and Real Mode
        if (IsKeyPressed(KEY_F) && Shared::editorMode && !Shared::popupOpen && !ImGui::GetIO().WantCaptureKeyboard){
            Shared::editorMode = false;//RealEditor
            Shared::setPlayerPos = true;
            DisableCursor();
        }
        else if (IsKeyPressed(KEY_F) && !Shared::editorMode && !Shared::popupOpen && !ImGui::GetIO().WantCaptureKeyboard) {
            Shared::editorMode = true;
            EnableCursor();
        }

        //Press P to switch from freecam to playerCam in real Editor Mode
        if(IsKeyPressed(KEY_P) && !Shared::editorMode && !Shared::popupOpen){
            freeCamMode = !freeCamMode;
            DisableCursor();
        }

        //----Update------
        if (Shared::editorMode)//TileEditor Updates
            engine.UpdateEditor();
        else if (!Shared::editorMode) {//Real Editor Updates
            if(freeCamMode)
                engine.UpdateReal(&Shared::free.cam);
            else {
                engine.UpdateReal(&Shared::player.cam);
                
            }
        }

        //----Draw------
        BeginDrawing();
        ClearBackground(BLACK);

        if (Shared::editorMode) {
            
            //GUI(2d Stuff)
            BeginMode2D(engine.tileEditor.cam);

            engine.DrawEditor();

            EndMode2D();
        }
        else if (!Shared::editorMode) {
            //Real(3d Stuff)
            if(freeCamMode)
                BeginMode3D(Shared::free.cam);
            else
                BeginMode3D(Shared::player.cam);

            engine.DrawReal();

            EndMode3D();

            //GUI FOR 3D
                
            DrawCircle(GetScreenWidth()/2, GetScreenHeight()/2, 1, WHITE); //Crosshair
        }

        engine.DrawGUI();

        //Imgui
        rlImGuiBegin();
        imgui.UpdateImguiEditor();
        imgui.UpdateImguiExitPopup(running);

        rlImGuiEnd();


        EndDrawing();
    }

    //Unload
    Importer::UnloadTextures();
    Importer::UnloadModels();
    Importer::UnloadSounds();
    Importer::UnloadMusic();
    Importer::UnloadShaders();


    rlImGuiShutdown();
	CloseAudioDevice();
    CloseWindow(); 

    return 0;
}