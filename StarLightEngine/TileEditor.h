#pragma once
#include <vector>
#include "raylib.h"
#include "Wall.h"



class TileEditor
{

private:
	int cols = 0;
	int rows = 0;

	const int worldCols = 2024;
	const int worldRows = 2024;

	Vector2 mouse = GetMousePosition();
	Wall pendingWall;
	Wall placeHolder;

public:
	std::vector<Wall> walls;

	const int tileSize = 16;

	Camera2D cam;
	int panSpeed = 200;

	Vector2 oldCamOffset = { 0 };

	TileEditor();
	void InitCamera();
	void Update();
	void UpdateCam();
	void UpdateWall();
	void DrawWall();
	void Draw();
};



//-------TILES VECTOR------> 0->Empty | 1->Full