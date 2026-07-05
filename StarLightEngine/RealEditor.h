#pragma once
#include "Shared.h"
#include <memory>

class RealEditor
{
private:
	int moveSpeed = 125;
	float mouseSens = 0.1f;

	float yaw = 0.0f;
	float pitch = 0.0f;
	Vector3 forward;

	Ramp* selectedRamp = nullptr;
	Wall* selectedWall = nullptr;

public:
	//3d Camera
	Camera3D cam;

	RealEditor();
	void InitCam();

	void Update();
	void UpdateCam();
	void UpdateSector();
	void UpdateWall();
	void UpdateRamp();
	void UpdateWallFeatures();
	void UpdateDoor();

	void DrawDoorSwitch();
	void DrawWall();
	void DrawSector();
	void DrawRamp();
	void Draw();

	//Helper
	bool WallCloserThanSector();

};

