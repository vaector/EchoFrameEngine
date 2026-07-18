#include "Engine.h"
#include "Importer.h"

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

void Engine::UpdateGui()
{

	//Update Engine Entities
	tileEditor.Update();
}

void Engine::UpdateReal(Camera3D* cam)
{
	realEditor.Update(cam);
}

void Engine::DrawReal()
{
	realEditor.Draw();
	DrawModel(Shared::models["green_crate1"], {1024,0,1024},60, WHITE);
}

void Engine::DrawGui()
{
	//Draw Engine Entities
	tileEditor.Draw();
}
