#pragma once
#include <vector>
#include "raylib.h"
#include "Wall.h"
#include "Shared.h"
#include "Ramp.h"

class TileEditor
{

private:
	int cols = 0;
	int rows = 0;

	const int worldCols = 2024;
	const int worldRows = 2024;

	Vector2 mouse = GetMousePosition();
	Wall pendingWall;
	Wall placeHolderWall;

	Ramp pendingRamp;
	Ramp placeHolderRamp;

	std::vector<Vector2*> draggingVertices;
	std::vector<Vector2*> draggingSectorVertices;
public:

	const int tileSize = 16;

	Camera2D cam;
	int panSpeed = 200;

	Vector2 oldCamOffset = { 0 };

	TileEditor();
	void InitCamera();
	void Update();
	void UpdateCam();
	void UpdateWall();
	bool UpdateWallVertices();
	void UpdateSector();
	void UpdateRamp();


	void DrawWall();
	void DrawSector();
	void DrawRamp();
	void Draw();
};

//-------UpdateSector----- Press "C" to build the sector