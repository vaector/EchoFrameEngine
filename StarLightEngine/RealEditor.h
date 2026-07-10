#pragma once
#include "Shared.h"
#include <memory>

class RealEditor
{
private:

	float mouseSens = 0.1f;

	//FreeCam
	int freeMoveSpeed = 125;

	float freeYaw = 0.0f;
	float freePitch = 0.0f;
	Vector3 freeForward;

	//PlayerCam
	int playerMoveSpeed = 125;

	float playerYaw = 0.0f;
	float playerPitch = 0.0f;
	Vector3 playerForward;
	float gravity = 150.0f;
	float jump = 100.0f;
	bool isGround = false;
	float verticalVelocity = 0.0f;
	float playerHeight = 100.0f;

	//---------------
	Ramp* selectedRamp = nullptr;
	Wall* selectedWall = nullptr;

public:
	//3d Camera
	Camera3D freeCam;
	Camera3D playerCam;

	RealEditor();
	void InitCam();

	void Update(Camera3D* cam);
	void UpdateCam(Camera3D* cam);
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

	//Helper
	bool WallCloserThanSector();
	bool CheckCollisionWall(Camera3D* cam, Vector3* nextPos);
	bool CheckCollisionSector(Camera3D* cam, Vector3* nextPos);
	bool CheckCollisionRamp(Camera3D* cam, Vector3* nextPos);
	bool PointInSector(Vector2 point, const std::vector<Vector2>& poly);
};

//Functions with "EDIT" are for the free Camera-> Editing
//Functions with "PLAYER" are for the player Camera-> Gameplay