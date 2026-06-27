#include "RealEditor.h"
#include "raymath.h"
#include "rlgl.h"

RealEditor::RealEditor()
{
	InitCam();
	wallTexture = Shared::textures["tile1"];

}

void RealEditor::InitCam()
{
	cam.fovy = 60;
	cam.position = { 20,0,20};
	cam.projection = CAMERA_PERSPECTIVE;
	cam.target = { 0 };
	cam.up = { 0.0f, 1.0f, 0.0f };

	forward = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
	yaw = atan2f(forward.z, forward.x);
	pitch = asinf(forward.y);
}

void RealEditor::Update()
{
	
	//Update Camera
	UpdateCam();

	//Update walls
	UpdateWall();
}

void RealEditor::UpdateCam()
{
	float dt = GetFrameTime();

	Vector3 move = { 0 };

	//Camera
	Vector2 mouseDt = GetMouseDelta();

	yaw += -mouseDt.x * (mouseSens * DEG2RAD);
	pitch += -mouseDt.y * (mouseSens * DEG2RAD);

	pitch = Clamp(pitch, -89.0f * DEG2RAD, 89.0f * DEG2RAD);

	forward.x = cosf(pitch) * sinf(yaw);
	forward.y = sinf(pitch);
	forward.z = cosf(pitch) * cosf(yaw);

	forward = Vector3Normalize(forward);

	Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, {0,1,0}));
	Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

	//Movement
	if (IsKeyDown(KEY_W))
		move = Vector3Add(move,Vector3Scale(forward, moveSpeed*dt));
	if(IsKeyDown(KEY_S))
		move = Vector3Add(move, Vector3Scale(forward, -moveSpeed * dt));
	if (IsKeyDown(KEY_D))
		move = Vector3Add(move, Vector3Scale(right, moveSpeed * dt));
	if (IsKeyDown(KEY_A))
		move = Vector3Add(move, Vector3Scale(right, -moveSpeed * dt));
	if (IsKeyDown(KEY_E))
		move = Vector3Add(move, Vector3Scale(up, moveSpeed * dt));
	if (IsKeyDown(KEY_Q))
		move = Vector3Add(move, Vector3Scale(up, -moveSpeed * dt));


	cam.position = Vector3Add(cam.position, move);
	cam.target = Vector3Add(cam.position, forward);

	

}

void RealEditor::UpdateWall()
{
	
}
void RealEditor::DrawWall()
{
	//Disable back face culling
	rlDisableBackfaceCulling();

	for (Wall& wall : Shared::walls) {

		Vector3 topLeft = { wall.startPos.x, Shared::wallHeight, wall.startPos.y };
		Vector3 bottomLeft = { wall.startPos.x, 0.0f,       wall.startPos.y };
		Vector3 topRight = { wall.endPos.x,   Shared::wallHeight, wall.endPos.y };
		Vector3 bottomRight = { wall.endPos.x,   0.0f,       wall.endPos.y };

		float wallLength = Vector2Distance(wall.startPos, wall.endPos);
		float tileX = wallLength / 32.0f; 
		float tileY = Shared::wallHeight / 32.0f;

		rlSetTexture(wall.texture->id);
		rlBegin(RL_QUADS);

		rlColor4ub(255, 255, 255, 255);

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(topRight.x, topRight.y, topRight.z);

		rlEnd();
		rlSetTexture(0);
	}
}

void RealEditor::Draw()
{
	//Draw Walls
	DrawWall();
}
