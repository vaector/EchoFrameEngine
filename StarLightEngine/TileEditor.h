#pragma once
#include <vector>
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

	Wall* selectedWall = nullptr;

	bool WallMode = true;
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
	void UpdateDoor();
	void UpdateDoorSwitch();
	void UpdateSelectedWall();
	void UpdatePlayerPos();

	bool RemoveWall();
	bool RemoveSector();
	bool RemoveRamp();
	bool RemoveDoorSwitch();

	void DrawPlayerPos();
	void DrawDoorSwitch();
	void DrawWall();
	void DrawSector();
	void DrawRamp();
	void Draw();

	//Helper
	Vector2 ClosestPointOnLineSegment(Vector2 p, Vector2 a, Vector2 b);
	bool PointInSector(Vector2 point, const std::vector<Vector2>& poly);
	bool pointInWall(Vector2 point, Vector2 startPos, Vector2 endPos);
};

//-------UpdateSector----- Press "C" to build the sector
//-------UpdateRamp------- Press "R" to build the ramp
//-------DELETE----------- Right click to delete objects(walls, ramps, door switches)