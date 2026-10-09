#include "Engine.h"
#include "Importer.h"
#include "DialougeSystem.h"

Engine::Engine()
{
	// Initialize other engine components here
	this->is_running = true;
	
}

Engine::~Engine()
{
	// Clean up resources here
	this->is_running = false;
}

void Engine::UpdateEditor()
{

	//Update Engine Entities
	tileEditor.UpdateEditor();
}

void Engine::UpdateReal(Camera3D* cam)
{
	realEditor.Update(cam);
}

void Engine::DrawReal()
{
	realEditor.Draw();
}

void Engine::DrawEditor()
{
	//Draw Engine Entities
	tileEditor.Draw();
}

void Engine::DrawGUI()
{
	DrawFPS(200, 0);//FPS

	if (Shared::dialougeMode) {
		DialougeSystem::DrawDialouge();//Dialouge Box Draw
		DialougeSystem::RenderDialougeTexts(DialougeSystem::selectedDialougeNpcName, DialougeSystem::selectedDialougeName);//Dialouge Text
	}

}
