#pragma
#include "raylib.h"

struct Player {
	Camera3D cam;
	int moveSpeed = 150;

	float yaw = 0.0f;
	float pitch = 0.0f;
	Vector3 forward;
	float gravity = 512.0f;
	float jump = 250.0f;
	bool isGround = false;
	float verticalVelocity = 0.0f;
	float height = 150.0f;
	float radius = 8.0f;//Width of the palayer
	BoundingBox collider;
	float bottom;
	float top;
};

struct Free {
	Camera3D cam;
	int moveSpeed = 175;

	float yaw = 0.0f;
	float pitch = 0.0f;
	Vector3 forward;
};