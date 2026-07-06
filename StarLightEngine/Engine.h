#pragma once
#include "TileEditor.h"
#include "RealEditor.h"

class Engine
{

public:
	//Engine specifics
	bool is_running = false;

	//Tile Editor specifics
	TileEditor tileEditor;

	//Real 3D Editor
	RealEditor realEditor;

	

	Engine();
	~Engine();
	void UpdateGui();
	void UpdateReal(Camera3D* cam);
	void DrawReal();
	void DrawGui();


};

//--UPDATES----> UpdateReal-> Updating 3d from RealEditor | UpdateGui-> Updating 2d from TileEditor
//--Draws----> DrawReal-> Drawing 3d from RealEditor | DrawGui-> Draw 2d from TileEditor

