#pragma once
#include "Shared.h"
#include <memory>

class RealEditor
{
private:

	float mouseSens = 0.1f;

	bool freeCamToggle = true;//If true then free cam can be moved with mouse, otherwise if false then the free cam does not move(Camera Rotation wise).
	bool camMode = true;//True = FreeCam
	//---------------
	Ramp* selectedRamp = nullptr;
	Wall* selectedWall = nullptr;
	Sector* selectedFloor = nullptr;
	Sector* selectedCeiling = nullptr;

public:

	RealEditor();

	//Initialization/Create
	void InitCam();

	void Update(Camera3D* cam);
	void UpdateCam(Camera3D* cam);
	void UpdatePlayerHeight(Camera3D* cam, Vector3* nextPos);
	void UpdateSector();
	void UpdateWall();
	void UpdateRamp();
	void UpdateWallFeatures();
	void EditDoor();
	void PlayerDoor();

	void DrawDoorSwitch();
	void DrawWall();
	void DrawSector();
	void DrawRamp();
	void Draw();
	       
	//Entities
	void UpdateWorldObject();
	void CreateWorldObject();
	void DrawWorldObject(Shader activeShader);

	//NPC
	void CreateNpc();
	void DrawNpc(Shader activeShader);
	void UpdateNpc();

	//Helper
	bool WallCloserThanSector();
	bool CheckCollisionWall(Camera3D* cam, Vector3* nextPos);
	bool CheckCollisionSector(Camera3D* cam, Vector3* nextPos);
	bool CheckCollisionRamp(Camera3D* cam, Vector3* nextPos);
	bool PointInSector(Vector2 point, const std::vector<Vector2>& poly);
	bool CheckLineRectCollision(Vector2 lineStart, Vector2 lineEnd, Rectangle rect);
	bool CheckCollisionPlayerEntityXZ(Camera3D* cam, Vector3* nextPos);
	bool CheckCollisionPlayerEntityY(Camera3D* cam, Vector3* nextPos);

};

//Functions with "EDIT" are for the free Camera-> Editing
//Functions with "PLAYER" are for the player Camera-> Gameplay

//Press "TAB" to lock/unlock the cursor in the Real Editor Mode
//Press "F" to switch between Real Editor Mode and Tile Editor Mode
//Press "P" to switch between Free Camera and Player Camera in the Real Editor Mode
//Press "SPACE" to jump in the Player Camera Mode
//Hold "C" to move Entities and editable objects(Also to apply other editing featueres like scaling)
//Press "Up/Down Arrow" to change the y Position of Entities and editable objects.
//Press "Left/Right Arrow" to change scale.