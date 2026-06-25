#pragma once
#include "TileEditor.h"

class Engine
{

public:
	//Engine specifics
	bool is_running = false;

	//Tile Editor specifics
	TileEditor tileEditor;

	Engine();
	~Engine();
	void UpdateGui();
	void DrawGui();

};

