#include "Engine.h"
#include "Importer.h"
#include "DialougeSystem.h"
#include "Game.h"

Engine::Engine()
{
	// Initialize other engine components here
	this->is_running = true;
	Game::Start();
}

Engine::~Engine()
{
	// Clean up resources here
	this->is_running = false;
}

void Engine::UpdateEditor()
{

	//Update Editor
	tileEditor.UpdateEditor();
}

void Engine::UpdateReal(Camera3D* cam)
{
	//Update Real Editor and Game
	realEditor.Update(cam);
	Game::Update();
}

void Engine::DrawReal()
{

	//Draw Real Editor and Game
	realEditor.Draw();
	Game::Draw();
}

void Engine::DrawEditor()
{
	//Draw Engine Entities
	tileEditor.Draw();
}

void Engine::DrawGUI()
{
	DrawFPS(GetScreenWidth()/2, 0);//FPS

	if (Shared::dialougeMode) {
		DialougeSystem::DrawDialouge();//Dialouge Box Draw
		DialougeSystem::RenderDialougeTexts(DialougeSystem::selectedDialougeNpcName, DialougeSystem::selectedDialougeName);//Dialouge Text
	}

}
