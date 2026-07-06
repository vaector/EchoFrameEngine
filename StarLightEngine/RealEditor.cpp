#include "RealEditor.h"
#include "raymath.h"
#include "rlgl.h"
#include <iostream>

RealEditor::RealEditor()
{
	InitCam();

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

	//Update Wall/Sector
	if (WallCloserThanSector())
		UpdateWall();
	else
		UpdateSector();

	//Update Wall Features
	UpdateWallFeatures();

	//Update Ramp
	UpdateRamp();

	UpdateDoor();
	
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
void RealEditor::UpdateSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, cam);

	Sector* closestFloorSector = nullptr;
	Sector* closestCeilingSector = nullptr;
	float closestFloorDist = FLT_MAX;
	float closestCeilingDist = FLT_MAX;

	for (auto& sector : Shared::sectors)
	{
		// floor
		for (int j = 1; j < sector.floorPoints.size() - 1; j++)
		{
			Vector3 p0 = { sector.floorPoints[0].x,   (float)(sector.floor * Shared::wallHeight), sector.floorPoints[0].y };
			Vector3 p1 = { sector.floorPoints[j].x,   (float)(sector.floor * Shared::wallHeight), sector.floorPoints[j].y };
			Vector3 p2 = { sector.floorPoints[j + 1].x, (float)(sector.floor * Shared::wallHeight), sector.floorPoints[j + 1].y };

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);
			if (col.hit && col.distance < closestFloorDist)
			{
				closestFloorDist = col.distance;
				closestFloorSector = &sector;
			}
		}

		// ceiling
		for (int j = 1; j < sector.ceilingPoints.size() - 1; j++)
		{
			Vector3 p0 = { sector.ceilingPoints[0].x,   (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight, sector.ceilingPoints[0].y };
			Vector3 p1 = { sector.ceilingPoints[j].x,   (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight, sector.ceilingPoints[j].y };
			Vector3 p2 = { sector.ceilingPoints[j + 1].x, (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight, sector.ceilingPoints[j + 1].y };

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);
			if (col.hit && col.distance < closestCeilingDist)
			{
				closestCeilingDist = col.distance;
				closestCeilingSector = &sector;
			}
		}
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		if (closestFloorSector && closestFloorDist < closestCeilingDist)
			closestFloorSector->floorTexture = Shared::selectedTexture;
		else if (closestCeilingSector)
			closestCeilingSector->ceilingTexture = Shared::selectedTexture;
	}
}
void RealEditor::UpdateWall()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, cam);

	Wall* closestWall = nullptr;
	float closestDist = FLT_MAX;

	for (auto& wall : Shared::walls)
	{
		Vector3 topLeft = { wall->startPos.x, (wall->wallHeight * wall->floor) + wall->wallHeight, wall->startPos.y };
		Vector3 bottomLeft = { wall->startPos.x,  wall->wallHeight * wall->floor,                    wall->startPos.y };
		Vector3 topRight = { wall->endPos.x,   (wall->wallHeight * wall->floor) + wall->wallHeight,  wall->endPos.y };
		Vector3 bottomRight = { wall->endPos.x,    wall->wallHeight * wall->floor,                     wall->endPos.y };

		RayCollision wallCol = GetRayCollisionQuad(ray, topLeft, bottomLeft, bottomRight, topRight);

		if (wallCol.hit && wallCol.distance < closestDist)
		{
			closestDist = wallCol.distance;
			closestWall = wall;

			//Select Wall
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
				selectedWall = wall;
		}
		
	}

	//Deselect Wall
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		selectedWall = nullptr;

	//Change texture
	if (closestWall && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		closestWall->texture = Shared::selectedTexture;
	}

}
void RealEditor::UpdateRamp()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, cam);

	for (auto& ramp: Shared::ramps) {

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });

		float lowY = (float)ramp.rampHeight * ramp.floor;
		float highY = (float)ramp.rampHeight * ramp.floor + ramp.rampHeight;

		Vector3 startLeft = Vector3Subtract({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 startRight = Vector3Add({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endLeft = Vector3Subtract({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endRight = Vector3Add({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));

		RayCollision rampCol = GetRayCollisionQuad(ray, startLeft, endLeft, endRight, startRight);

		if (rampCol.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			selectedRamp = &ramp;
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		selectedRamp = nullptr;

	if (selectedRamp != nullptr) {
		if (IsKeyDown(KEY_UP))
			selectedRamp->rampHeight += 1.0f;
		else if (IsKeyDown(KEY_DOWN))
			selectedRamp->rampHeight -= 1.0f;

		selectedRamp->rampHeight = Clamp(selectedRamp->rampHeight, 0.0f, Shared::wallHeight);
	}
}
void RealEditor::UpdateWallFeatures()
{
	//Select Wall(selectedWall)
	if (selectedWall != nullptr) {
		if (IsKeyDown(KEY_UP))
			selectedWall->wallHeight += 1.0f;
		else if (IsKeyDown(KEY_DOWN))
			selectedWall->wallHeight -= 1.0f;

		selectedWall->wallHeight = Clamp(selectedWall->wallHeight, 1.0f, Shared::wallHeight);
	}
}
void RealEditor::UpdateDoor()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, cam);

	static DoorSwitch* selectedDoorSwitch = nullptr;
	static Vector2 savedDir = { 0 };
	static Vector3 savedPerp = { 0 };
	static float sign = 1.0f;
	
	for (auto& d : Shared::doorSwitches)
	{

		//Find the closest wall in the same floor as the door switch to draw the door switch in front of it.
		static Wall* closestWall = nullptr;
		float clostseDist = FLT_MAX;
		for (auto& wall : Shared::walls) {
			if (wall->floor != d.floor) continue;
			float dist = Vector2Distance({ d.pos.x, d.pos.z }, { (wall->startPos.x + wall->endPos.x) / 2, (wall->startPos.y + wall->endPos.y) / 2 });
			if (dist < clostseDist) {
				clostseDist = dist;
				closestWall = wall;
			}
		}

		Vector2 dir = Vector2Normalize(Vector2Subtract(closestWall->endPos, closestWall->startPos));
		Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });
		Vector3 pos = Vector3Add({ d.pos.x, (d.floor * Shared::wallHeight) + d.pos.y, d.pos.z }, Vector3Scale(perp, 2.0f));

		Vector3 startLeft = { pos };
		Vector3 startRight = Vector3Add(pos, Vector3Scale({ dir.x, 0, dir.y }, 25));
		Vector3 endLeft = { pos.x, pos.y + 25, pos.z };
		Vector3 endRight = Vector3Add(endLeft, Vector3Scale({ dir.x, 0, dir.y }, 25));

		RayCollision dCol = GetRayCollisionQuad(ray, startLeft, endLeft, endRight, startRight);

		if (dCol.hit)
		{
			selectedDoorSwitch = &d;
			savedDir = dir;
			sign = (Vector3DotProduct(forward, perp) < 0.0f) ? 1.0f : -1.0f;
			savedPerp = perp;

			break;
		}
		else if(!dCol.hit && !IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			selectedDoorSwitch = nullptr;
	}

	if (selectedDoorSwitch != nullptr)
	{
		if (IsKeyPressed(KEY_R) && selectedDoorSwitch->linkedWall != nullptr)
			selectedDoorSwitch->linkedWall->isOpen = !selectedDoorSwitch->linkedWall->isOpen;

		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		{
			Vector2 mouseDelta = GetMouseDelta();

			selectedDoorSwitch->pos = Vector3Add(
				selectedDoorSwitch->pos,
				Vector3Scale({ savedDir.x, 0, savedDir.y }, mouseDelta.x * 0.25f * sign)
			);

			selectedDoorSwitch->pos = Vector3Add(
				selectedDoorSwitch->pos,
				Vector3Scale({ 0, 1, 0 }, mouseDelta.y * -0.25f)
			);

		}
		
		if (GetMouseWheelMove() != 0)
		{
			float wheelMove = GetMouseWheelMove();
			selectedDoorSwitch->pos = Vector3Add({ selectedDoorSwitch->pos.x, (selectedDoorSwitch->floor * Shared::wallHeight) + selectedDoorSwitch->pos.y, selectedDoorSwitch->pos.z }, Vector3Scale(savedPerp, wheelMove * sign));

		}

		//Deselect Door Switch
		if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			selectedDoorSwitch = nullptr;
	}

	float speed = 150.0f * GetFrameTime();

	for (auto& d : Shared::doorSwitches)
	{

		if(d.linkedWall == nullptr) continue;

		if (d.linkedWall->isOpen) {
			d.linkedWall->wallHeight -= speed;
		}
		else if(!d.linkedWall->isOpen){
			d.linkedWall->wallHeight += speed;
		}

		d.linkedWall->wallHeight = Clamp(d.linkedWall->wallHeight, 0.0f, Shared::wallHeight);
	}

}
void RealEditor::DrawDoorSwitch()
{

	for (auto& d : Shared::doorSwitches) {

		//Find the closest wall in the same floor as the door switch to draw the door switch in front of it.
		static Wall* closestWall = nullptr;
		float clostseDist = FLT_MAX;
		for (auto& wall : Shared::walls) {
			if (wall->floor != d.floor) continue;
			float dist = Vector2Distance({ d.pos.x, d.pos.z }, { (wall->startPos.x + wall->endPos.x) / 2, (wall->startPos.y + wall->endPos.y) / 2 });
			if(dist < clostseDist) {
				clostseDist = dist;
				closestWall = wall;
			}
		}

		Vector2 dir = Vector2Normalize(Vector2Subtract(closestWall->endPos, closestWall->startPos));
		Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });
		Vector3 pos = Vector3Add({ d.pos.x, (d.floor * Shared::wallHeight) + d.pos.y, d.pos.z }, Vector3Scale(perp, 2.0f));
		
		Vector3 startLeft = { pos };
		Vector3 startRight = Vector3Add({ pos.x, pos.y, pos.z }, Vector3Scale({ dir.x, 0, dir.y }, 25));
		Vector3 endLeft = { pos.x, pos.y + 25, pos.z };
		Vector3 endRight = Vector3Add({ pos.x, pos.y + 25, pos.z }, Vector3Scale({ dir.x, 0, dir.y }, 25));

		float tileX = 2.0f;
		float tileY = 2.0f;

		rlBegin(RL_QUADS);
		rlSetTexture(d.texture->id);

		rlColor4ub(255, 255, 255, 255);

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(startLeft.x, startLeft.y, startLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(endLeft.x, endLeft.y, endLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(endRight.x, endRight.y, endRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(startRight.x, startRight.y, startRight.z);

		rlEnd();
		rlSetTexture(0);
	}
}
void RealEditor::DrawWall()
{
	for (auto& wall : Shared::walls) {

		Vector3 topLeft = { wall->startPos.x, (wall->wallHeight * wall->floor) + wall->wallHeight, wall->startPos.y };
		Vector3 bottomLeft = { wall->startPos.x, wall->wallHeight * wall->floor,       wall->startPos.y };
		Vector3 topRight = { wall->endPos.x,   (wall->wallHeight *wall->floor) + wall->wallHeight, wall->endPos.y };
		Vector3 bottomRight = { wall->endPos.x,   wall->wallHeight * wall->floor,       wall->endPos.y };

		float wallLength = Vector2Distance(wall->startPos, wall->endPos);
		float tileX = wallLength / 32.0f;
		float tileY = wall->wallHeight / 32.0f;

		
		rlBegin(RL_QUADS);
		rlSetTexture(wall->texture->id);

		if (selectedWall == wall)
			rlColor4ub(255, 0, 0, 255);
		else
			rlColor4ub(255, 255, 255, 255);
		

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(topRight.x, topRight.y, topRight.z);

		rlEnd();
		rlSetTexture(0);
	}
}

void RealEditor::DrawSector()
{
	rlDisableDepthTest();
	for (auto& sector : Shared::sectors)
	{
		//Floor
		rlBegin(RL_TRIANGLES);
		rlSetTexture(sector.floorTexture->id);
		rlColor4ub(255, 255, 255, 255);
		for (int i = 1; i < sector.floorPoints.size() - 1; i++)
		{
			rlTexCoord2f(sector.floorPoints[0].x / 32.0f, sector.floorPoints[0].y / 32.0f);
			rlVertex3f(sector.floorPoints[0].x, sector.floor * Shared::wallHeight, sector.floorPoints[0].y);

			rlTexCoord2f(sector.floorPoints[i].x / 32.0f, sector.floorPoints[i].y / 32.0f);
			rlVertex3f(sector.floorPoints[i].x, sector.floor * Shared::wallHeight, sector.floorPoints[i].y);

			rlTexCoord2f(sector.floorPoints[i + 1].x / 32.0f, sector.floorPoints[i + 1].y / 32.0f);
			rlVertex3f(sector.floorPoints[i + 1].x, sector.floor * Shared::wallHeight, sector.floorPoints[i + 1].y);
		}
		rlEnd();
		rlSetTexture(0);
		
		//Ceiling
		rlBegin(RL_TRIANGLES);
		rlSetTexture(sector.ceilingTexture->id);
		rlColor4ub(255, 255, 255, 255);
		for (int i = 1; i < sector.ceilingPoints.size() - 1; i++)
		{
			rlTexCoord2f(sector.ceilingPoints[i + 1].x / 32.0f, sector.ceilingPoints[i + 1].y / 32.0f);
			rlVertex3f(sector.ceilingPoints[i + 1].x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[i + 1].y);

			rlTexCoord2f(sector.ceilingPoints[i].x / 32.0f, sector.ceilingPoints[i].y / 32.0f);
			rlVertex3f(sector.ceilingPoints[i].x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[i].y);

			rlTexCoord2f(sector.ceilingPoints[0].x / 32.0f, sector.ceilingPoints[0].y / 32.0f);
			rlVertex3f(sector.ceilingPoints[0].x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[0].y);
			
		}
		rlEnd();
		rlSetTexture(0);
	}
	rlEnableDepthTest();
}

void RealEditor::DrawRamp()
{
	for (auto& ramp : Shared::ramps) {

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });

		float lowY = (float)ramp.rampHeight * ramp.floor;
		float highY = (float)ramp.rampHeight * ramp.floor + ramp.rampHeight; 

		Vector3 startLeft = Vector3Subtract({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 startRight = Vector3Add({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endLeft = Vector3Subtract({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endRight = Vector3Add({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));

		float wallLength = Vector2Distance(ramp.startPos, ramp.endPos);
		float tileX = 12;
		float tileY = 12;

		if (selectedRamp != nullptr)
			rlEnableWireMode();

		rlBegin(RL_QUADS);
		rlSetTexture(ramp.texture->id);

		if (selectedRamp == &ramp)//Draw selected ramp in red
			rlColor4ub(255, 0, 0, 255);
		else
			rlColor4ub(255, 255, 255, 255);

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(endLeft.x, endLeft.y, endLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(startLeft.x, startLeft.y, startLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(startRight.x, startRight.y, startRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(endRight.x, endRight.y, endRight.z);

		rlEnd();
		rlSetTexture(0);

		rlDisableWireMode();
	}
}

void RealEditor::Draw()
{
	//Disable back face culling
	rlDisableBackfaceCulling();

	//Draw Walls
	DrawWall();

	//Draw Sector
	DrawSector();

	DrawRamp();

	DrawDoorSwitch();
}

bool RealEditor::WallCloserThanSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, cam);

	float wallDist = FLT_MAX;
	float sectorDist = FLT_MAX;

	for (auto& wall : Shared::walls)
	{
		Vector3 topLeft = { wall->startPos.x, (wall->wallHeight * wall->floor) + wall->wallHeight, wall->startPos.y };
		Vector3 bottomLeft = { wall->startPos.x,  wall->wallHeight * wall->floor,                    wall->startPos.y };
		Vector3 topRight = { wall->endPos.x,   (wall->wallHeight * wall->floor) + wall->wallHeight,  wall->endPos.y };
		Vector3 bottomRight = { wall->endPos.x,    wall->wallHeight * wall->floor,                   wall->endPos.y };

		RayCollision col = GetRayCollisionQuad(ray, topLeft, bottomLeft, bottomRight, topRight);
		if (col.hit && col.distance < wallDist)
			wallDist = col.distance;
	}

	for (auto& sector : Shared::sectors)
	{
		for (int j = 1; j < sector.floorPoints.size() - 1; j++)
		{
			Vector3 p0 = { sector.floorPoints[0].x,   (float)(sector.floor * Shared::wallHeight), sector.floorPoints[0].y };
			Vector3 p1 = { sector.floorPoints[j].x,   (float)(sector.floor * Shared::wallHeight), sector.floorPoints[j].y };
			Vector3 p2 = { sector.floorPoints[j + 1].x, (float)(sector.floor * Shared::wallHeight), sector.floorPoints[j + 1].y };

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);
			if (col.hit && col.distance < sectorDist)
				sectorDist = col.distance; 
		}
	}

	return wallDist < sectorDist;
}
