#pragma once
#include "Shared.h"

class RealEditor
{
private:
	int moveSpeed = 100;
	float mouseSens = 0.1f;

	float yaw = 0.0f;
	float pitch = 0.0f;
	Vector3 forward;


public:
	//3d Camera
	Camera3D cam;

	RealEditor();
	void InitCam();

	void Update();
	void UpdateCam();
	void UpdateSector();

	void DrawWall();
	void DrawSector();
	void Draw();

};

