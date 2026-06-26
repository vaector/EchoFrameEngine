#include "Engine.h"

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

void Engine::UpdateReal()
{
	realEditor.Update();
}

void Engine::DrawReal()
{
	realEditor.Draw();
}

void Engine::DrawGui()
{
	//Draw Engine Entities
	tileEditor.Draw();
}
