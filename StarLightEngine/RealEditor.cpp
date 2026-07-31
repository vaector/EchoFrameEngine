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

	//Free Cam
	freeCam.fovy = 60;
	freeCam.position = { 1024,50,1024};//1024
	freeCam.projection = CAMERA_PERSPECTIVE;
	freeCam.target = { 0 };
	freeCam.up = { 0.0f, 1.0f, 0.0f };

	freeForward = Vector3Normalize(Vector3Subtract(freeCam.target, freeCam.position));
	freeYaw = atan2f(freeForward.z, freeForward.x);
	freePitch = asinf(freeForward.y);

	//Player Cam
	playerCam.fovy = 60;
	playerCam.position = { 1024,50,1024 };
	playerCam.projection = CAMERA_PERSPECTIVE;
	playerCam.target = { 0 };
	playerCam.up = { 0.0f, 1.0f, 0.0f };

	playerForward = Vector3Normalize(Vector3Subtract(playerCam.target, playerCam.position));
	playerYaw = atan2f(playerForward.z, playerForward.x);
	playerPitch = asinf(playerForward.y);
}


void RealEditor::Update(Camera3D* cam)
{
	if(Shared::onImguiWindow) return;

	//Update Cursor Lock
	static bool cursorLocked = true;
	if(IsKeyPressed(KEY_TAB))
	{
		cursorLocked = !cursorLocked;
		if(cursorLocked) DisableCursor();
		else EnableCursor();
	}

	if(!cursorLocked) return;
	
	//Update Camera
	UpdateCam(cam);

	//Update Wall/Sector
	if (WallCloserThanSector())
		UpdateWall();
	else
		UpdateSector();

	//Update Wall Features
	UpdateWallFeatures();

	//Update Ramp
	UpdateRamp();

	EditDoor();
	PlayerDoor();

	CreateEntity();
	UpdateEntity();

	CreateNpc();
	RemoveNpc();

	int c = 1; bool loop = true;
	for (auto& [name, npc] : Shared::npcs)
		npc.playAnim(c, loop);

}

void RealEditor::UpdateCam(Camera3D* cam)
{
	float dt = GetFrameTime();
	Vector3 move = { 0 };
	Vector2 mouseDt = GetMouseDelta();
	
	if (cam == &freeCam) {
		camMode = true;
		//Camera

		if (freeCamToggle) {
			freeYaw += -mouseDt.x * (mouseSens * DEG2RAD);
			freePitch += -mouseDt.y * (mouseSens * DEG2RAD);
		}
		freePitch = Clamp(freePitch, -89.0f * DEG2RAD, 89.0f * DEG2RAD);

		freeForward.x = cosf(freePitch) * sinf(freeYaw);
		freeForward.y = sinf(freePitch);
		freeForward.z = cosf(freePitch) * cosf(freeYaw);

		freeForward = Vector3Normalize(freeForward);

		Vector3 right = Vector3Normalize(Vector3CrossProduct(freeForward, { 0,1,0 }));
		Vector3 up = Vector3Normalize(Vector3CrossProduct(right, freeForward));

		//Movement
		if (IsKeyDown(KEY_W))
			move = Vector3Add(move, Vector3Scale(freeForward, freeMoveSpeed * dt));
		if (IsKeyDown(KEY_S))
			move = Vector3Add(move, Vector3Scale(freeForward, -freeMoveSpeed * dt));
		if (IsKeyDown(KEY_D))
			move = Vector3Add(move, Vector3Scale(right, freeMoveSpeed * dt));
		if (IsKeyDown(KEY_A))
			move = Vector3Add(move, Vector3Scale(right, -freeMoveSpeed * dt));
		if (IsKeyDown(KEY_E))
			move = Vector3Add(move, Vector3Scale(up, freeMoveSpeed * dt));
		if (IsKeyDown(KEY_Q))
			move = Vector3Add(move, Vector3Scale(up, -freeMoveSpeed * dt));


		freeCam.position = Vector3Add(freeCam.position, move);
		freeCam.target = Vector3Add(freeCam.position, freeForward);
	}
	else if (cam == &playerCam) {
		camMode = false;
		//Update Position from TileEditor
		if (Shared::setPlayerPos)
		{
			playerCam.position.x = Shared::playerPos.x;
			playerCam.position.y = Shared::playerPos.y;
			playerCam.position.z = Shared::playerPos.z;
			Shared::setPlayerPos = false;
		}

		//Camera
		playerYaw += -mouseDt.x * (mouseSens * DEG2RAD);
		playerPitch += -mouseDt.y * (mouseSens * DEG2RAD);

		playerPitch = Clamp(playerPitch, -89.0f * DEG2RAD, 89.0f * DEG2RAD);

		playerForward.x = cosf(playerPitch) * sinf(playerYaw);
		playerForward.y = sinf(playerPitch);
		playerForward.z = cosf(playerPitch) * cosf(playerYaw);

		playerForward = Vector3Normalize(playerForward);

		Vector3 right = Vector3Normalize(Vector3CrossProduct(playerForward, { 0,1,0 }));
		Vector3 up = Vector3Normalize(Vector3CrossProduct(right, playerForward));

		//Movement
		if (IsKeyDown(KEY_W))
			move = Vector3Add(move, Vector3Scale(playerForward, playerMoveSpeed * dt));
		if (IsKeyDown(KEY_S))
			move = Vector3Add(move, Vector3Scale(playerForward, -playerMoveSpeed * dt));
		if (IsKeyDown(KEY_D))
			move = Vector3Add(move, Vector3Scale(right, playerMoveSpeed * dt));
		if (IsKeyDown(KEY_A))
			move = Vector3Add(move, Vector3Scale(right, -playerMoveSpeed * dt));


		//COLLISION
		Vector3 moveX = { move.x, 0, 0 };
		Vector3 moveY = { 0, move.y, 0 };
		Vector3 moveZ = { 0, 0, move.z };

		Vector3 nextPosX = Vector3Add(playerCam.position, moveX);
		Vector3 nextPosY = Vector3Add(playerCam.position, moveY);
		Vector3 nextPosZ = Vector3Add(playerCam.position, moveZ);
		

		//Check XZ Wall Collision
		if (CheckCollisionWall(cam, &nextPosX))                         
			move.x = 0;                                             
		if (CheckCollisionWall(cam, &nextPosZ))
			move.z = 0;


		//Check collision with entities
		if (CheckCollisionPlayerEntityXZ(&playerCam, &nextPosX)) {
			move.x = 0;
		}
		if (CheckCollisionPlayerEntityXZ(&playerCam, &nextPosZ)) {
			move.z = 0;
		}
		

		// Gravity
		if (!isGround)
			verticalVelocity -= gravity * dt;

		// Jump
		if (IsKeyPressed(KEY_SPACE) && isGround)
		{
			verticalVelocity = jump;
			isGround = false;
		}

		// Apply vertical velocity
		move.y = verticalVelocity * dt;

		// Calculate the next Y position AFTER move.y is updated
		nextPosY = playerCam.position;
		nextPosY.y += move.y;

		UpdatePlayerHeight(&playerCam, &nextPosY);
		// Check collision
		if (CheckCollisionSector(cam, &nextPosY) || CheckCollisionRamp(cam, &nextPosY))
		{
			verticalVelocity = 0.0f;
			isGround = true;

			move.y = nextPosY.y - playerCam.position.y;
		}
		else
		{
			isGround = false;
		}

		if (CheckCollisionPlayerEntityY(&playerCam, &nextPosY)) {
			verticalVelocity = 0;
			move.y = nextPosY.y - playerCam.position.y;
			isGround = true;
		}

		// Apply updated position and target
		playerCam.position = Vector3Add(playerCam.position, move);
		playerCam.target = Vector3Add(playerCam.position, playerForward);
	}

}
void RealEditor::UpdatePlayerHeight(Camera3D* cam, Vector3* nextPos)
{
	playerBottom = nextPos->y - playerHeight / 2;
	playerTop = nextPos->y + playerHeight / 2;
}
void RealEditor::UpdateSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);


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
			if (col.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && col.distance < closestFloorDist)
			{
				closestFloorDist = col.distance;
				selectedFloor = &sector;
			}
		}

		// ceiling
		for (int j = 1; j < sector.ceilingPoints.size() - 1; j++)
		{
			Vector3 p0 = { sector.ceilingPoints[0].x,   (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[0].y };
			Vector3 p1 = { sector.ceilingPoints[j].x,   (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[j].y };
			Vector3 p2 = { sector.ceilingPoints[j + 1].x, (float)(sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, sector.ceilingPoints[j + 1].y };

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);
			if (col.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && col.distance < closestCeilingDist)
			{
				closestCeilingDist = col.distance;
				selectedCeiling = &sector;
			}
		}
	}

	//Deselect
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		selectedCeiling = nullptr;
		selectedFloor = nullptr;
	}

	if (selectedFloor && closestFloorDist < closestCeilingDist)
		selectedFloor->floorTexture = Shared::selectedTexture;
	else if (selectedCeiling)
		selectedCeiling->ceilingTexture = Shared::selectedTexture;
}
void RealEditor::UpdateWall()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

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
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

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
void RealEditor::EditDoor()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

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

		//Select
		if (dCol.hit)
		{
			selectedDoorSwitch = &d;
			savedDir = dir;
			sign = (Vector3DotProduct(freeForward, perp) < 0.0f) ? 1.0f : -1.0f;
			savedPerp = perp;

			break;
		}
		//Deselect
		else if(!dCol.hit && !IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			selectedDoorSwitch = nullptr;
	}

	if (selectedDoorSwitch != nullptr)
	{
		if (IsKeyPressed(KEY_R) && selectedDoorSwitch->linkedWall != nullptr )
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
void RealEditor::PlayerDoor()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, playerCam);

	static DoorSwitch* selectedDoorSwitch = nullptr;
	static Vector2 savedDir = { 0 };
	static Vector3 savedPerp = { 0 };
	static float sign = 1.0f;


	Wall* closestPlayerWall = nullptr;
	float closestPlayerDist = FLT_MAX;
	for (auto& wall : Shared::walls)
	{
		Vector3 topLeft = { wall->startPos.x, (wall->wallHeight * wall->floor) + wall->wallHeight, wall->startPos.y };
		Vector3 bottomLeft = { wall->startPos.x,  wall->wallHeight * wall->floor,                    wall->startPos.y };
		Vector3 topRight = { wall->endPos.x,   (wall->wallHeight * wall->floor) + wall->wallHeight,  wall->endPos.y };
		Vector3 bottomRight = { wall->endPos.x,    wall->wallHeight * wall->floor,                     wall->endPos.y };

		RayCollision wallCol = GetRayCollisionQuad(ray, topLeft, bottomLeft, bottomRight, topRight);

		if (wallCol.hit && wallCol.distance < closestPlayerDist)
		{
			closestPlayerDist = wallCol.distance;
			closestPlayerWall = wall;
		}

	}


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

		//Select
		if (dCol.hit)
		{
			if (closestPlayerWall && closestPlayerDist < dCol.distance)
				continue;

			selectedDoorSwitch = &d;
			savedDir = dir;
			sign = (Vector3DotProduct(freeForward, perp) < 0.0f) ? 1.0f : -1.0f;
			savedPerp = perp;

			break;
		}
		//Deselect
		else if (!dCol.hit && !IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			selectedDoorSwitch = nullptr;
	}

	if (selectedDoorSwitch != nullptr)
	{
		if (IsKeyPressed(KEY_R) && selectedDoorSwitch->linkedWall != nullptr)
			selectedDoorSwitch->linkedWall->isOpen = !selectedDoorSwitch->linkedWall->isOpen;

		//Deselect Door Switch
		if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
			selectedDoorSwitch = nullptr;
	}

	float speed = 150.0f * GetFrameTime();

	for (auto& d : Shared::doorSwitches)
	{

		if (d.linkedWall == nullptr) continue;

		if (d.linkedWall->isOpen) {
			d.linkedWall->wallHeight -= speed;
		}
		else if (!d.linkedWall->isOpen) {
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
			rlColor4ub(255, 0, 0, 150);
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
		// Floor
		if (sector.hasFloor)
		{
			rlBegin(RL_TRIANGLES);
			rlSetTexture(sector.floorTexture->id);
			
			if (selectedFloor == &sector)
				rlColor4ub(255, 0, 0, 150);
			else
				rlColor4ub(255, 255, 255, 255);

			for (int i = 0; i + 2 < sector.indices.size(); i += 3)
			{
				const Vector2& p0 = sector.floorPoints[sector.indices[i]];
				const Vector2& p1 = sector.floorPoints[sector.indices[i + 2]];
				const Vector2& p2 = sector.floorPoints[sector.indices[i + 1]];

				rlTexCoord2f(p0.x / 32.0f, p0.y / 32.0f);
				rlVertex3f(p0.x, sector.floor * Shared::wallHeight, p0.y);

				rlTexCoord2f(p1.x / 32.0f, p1.y / 32.0f);
				rlVertex3f(p1.x, sector.floor * Shared::wallHeight, p1.y);

				rlTexCoord2f(p2.x / 32.0f, p2.y / 32.0f);
				rlVertex3f(p2.x, sector.floor * Shared::wallHeight, p2.y);
			}

			rlEnd();
			rlSetTexture(0);
		}

		// Ceiling
		if (sector.hasCeiling)
		{
			rlBegin(RL_TRIANGLES);
			rlSetTexture(sector.ceilingTexture->id);
			
			if (selectedCeiling == &sector)
				rlColor4ub(255, 0, 0, 150);
			else
				rlColor4ub(255, 255, 255, 255);

			for (int i = 0; i + 2 < sector.indices.size(); i += 3)
			{
				const Vector2& p0 = sector.ceilingPoints[sector.indices[i]];
				const Vector2& p1 = sector.ceilingPoints[sector.indices[i + 1]];
				const Vector2& p2 = sector.ceilingPoints[sector.indices[i + 2]];

				// Reverse winding so the ceiling faces downward
				rlTexCoord2f(p2.x / 32.0f, p2.y / 32.0f);
				rlVertex3f(p2.x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, p2.y);

				rlTexCoord2f(p1.x / 32.0f, p1.y / 32.0f);
				rlVertex3f(p1.x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, p1.y);

				rlTexCoord2f(p0.x / 32.0f, p0.y / 32.0f);
				rlVertex3f(p0.x, (sector.floor * Shared::wallHeight) + Shared::wallHeight - 1.0f, p0.y);
			}

			rlEnd();
			rlSetTexture(0);
		}
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

	DrawEntity();

	DrawNpc();
}

void RealEditor::UpdateEntity()
{
	if (camMode == false) return;//Player Cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

	//Move entities
	for (auto& entity : Shared::entities) {
		RayCollision hit = GetRayCollisionBox(ray, entity.collider);
		if (hit.hit && IsKeyPressed(KEY_C)) {
			selectedEntity = &entity;
			freeCamToggle = false;
		}
	}

	//If an entity is selected
	if (selectedEntity != nullptr) {
		//Move the entity and its collider
		Vector3 perp = Vector3CrossProduct(freeForward, { 0,1,0 });
		Vector3 delta =Vector3Normalize(Vector3Add(Vector3Scale(perp, GetMouseDelta().x), Vector3Scale(freeForward, -GetMouseDelta().y)));
		Vector3 move = Vector3Scale(delta, 2.5f);
		
		if (IsKeyPressed(KEY_UP))
			move.y = 4.0f;
		else if (IsKeyPressed(KEY_DOWN)) 
			move.y = -4.0f;
		else
			move.y = 0.0f;

		//Updating Y
		selectedEntity->position = Vector3Add(selectedEntity->position, {0,move.y, 0});
		selectedEntity->collider = GetUpdatedBoundingBox(selectedEntity->originalCollider, selectedEntity->position, selectedEntity->scale);


		//Updating XZ and check Wall collisons

		selectedEntity->position = Vector3Add(selectedEntity->position, { move.x, 0, move.z});
		selectedEntity->collider = GetUpdatedBoundingBox(selectedEntity->originalCollider, selectedEntity->position, selectedEntity->scale);

		//Convert the collider into 2d XZ Plane to check if it collides with a wall.
		Rectangle rect;
		rect.x = selectedEntity->collider.min.x;
		rect.y = selectedEntity->collider.min.z;

		rect.width = selectedEntity->collider.max.x - selectedEntity->collider.min.x;
		rect.height = selectedEntity->collider.max.z - selectedEntity->collider.min.z;

		for (auto& wall : Shared::walls) {
			if (CheckLineRectCollision(wall->startPos, wall->endPos, rect))
			{
				selectedEntity->position = Vector3Subtract(selectedEntity->position, { move.x, 0, move.z });
				selectedEntity->collider = GetUpdatedBoundingBox(selectedEntity->originalCollider, selectedEntity->position, selectedEntity->scale);

			}
		}


		//Change Scale while pressing arrow keys
		if (IsKeyPressed(KEY_RIGHT)) {
			selectedEntity->scale = Vector3Add(selectedEntity->scale, {2,2,2});
		}
		else if (IsKeyPressed(KEY_LEFT)) {
			selectedEntity->scale = Vector3Subtract(selectedEntity->scale, {2,2,2});
		}

		selectedEntity->scale = Vector3Clamp(selectedEntity->scale, { 1, 1, 1 }, { 200,100, 100 });
		selectedEntity->collider = GetUpdatedBoundingBox(selectedEntity->originalCollider, selectedEntity->position, selectedEntity->scale);


	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		selectedEntity = nullptr;
		freeCamToggle = true;
	}

	//Remove Entity
	for (int i = 0; i < Shared::entities.size(); i++) {
		auto& entity = Shared::entities[i];

		RayCollision hit = GetRayCollisionBox(ray, entity.collider);
		if (hit.hit && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Shared::entities.erase(Shared::entities.begin() +i);
			selectedEntity = nullptr;
			freeCamToggle = true;
			break;
		}
	}

	//Deselect entity
	if (selectedEntity != nullptr && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		selectedEntity = nullptr;
		freeCamToggle = true;

	}

}

void RealEditor::CreateEntity()
{
	if (Shared::selectedModelName == nullptr || selectedFloor == nullptr || Shared::npcMode) return;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Entity newEntity;
		newEntity.id = Shared::entities.size() + 1;
		
		newEntity.modelName = *Shared::selectedModelName;

		Vector3 direction = Vector3Normalize(Vector3Subtract(freeCam.target, freeCam.position));
		Vector3 spawnPos = Vector3Add(freeCam.position, Vector3Scale(direction, 50.0f)); // Spawn 5 units in front of the camera

		newEntity.position = { spawnPos.x, (float)selectedFloor->floor * Shared::wallHeight, spawnPos.z };

		BoundingBox box = GetModelBoundingBox(Shared::models[newEntity.modelName]);
		newEntity.originalCollider = box;

		box.min.x *= newEntity.scale.x;
		box.min.y *= newEntity.scale.y;
		box.min.z *= newEntity.scale.z;

		box.max.x *= newEntity.scale.x;
		box.max.y *= newEntity.scale.y;
		box.max.z *= newEntity.scale.z;


		box.min = Vector3Add(box.min, newEntity.position);
		box.max = Vector3Add(box.max, newEntity.position);

		newEntity.collider = box;

		Shared::entities.push_back(newEntity);
		Shared::selectedModelName = nullptr;
	}
	
}

void RealEditor::DrawEntity()
{
	for (auto& entity : Shared::entities) {
		DrawModelEx(Shared::models[entity.modelName], entity.position, { 0,0,0 }, 0, entity.scale, WHITE);
		DrawBoundingBox(entity.collider, RED);
	}
}

void RealEditor::CreateNpc()
{
	if (!camMode || !Shared::npcMode || !camMode) return;//If not free Cam

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && Shared::selectedModelName != nullptr && selectedFloor != nullptr) {
		NPC npc;
		npc.modelName = *Shared::selectedModelName;
		npc.name = *Shared::selectedModelName;

		std::string path = "models/";
		path += npc.name;
		path += ".glb";


		npc.scale = { 50,50,50 };
		npc.model = Shared::models[npc.modelName];

		//Position
		Vector3 direction = Vector3Normalize(Vector3Subtract(freeCam.target, freeCam.position));
		Vector3 spawnPos = Vector3Add(freeCam.position, Vector3Scale(direction, 50.0f)); // Spawn 5 units in front of the camera
		npc.position = { spawnPos.x, (float)selectedFloor->floor * Shared::wallHeight, spawnPos.z };

		//Collider
		BoundingBox box = GetModelBoundingBox(Shared::models[npc.modelName]);
		npc.scale = Vector3Scale(npc.scale, 0.01);
		box.min.x *= npc.scale.x;
		box.min.y *= npc.scale.y;
		box.min.z *= npc.scale.z;

		box.max.x *= npc.scale.x;
		box.max.y *= npc.scale.y;
		box.max.z *= npc.scale.z;

		box.min = Vector3Add(box.min, npc.position);
		box.max = Vector3Add(box.max, npc.position);

		npc.collider = box;
		

		int animCount = 0;
		npc.modelAnim = LoadModelAnimations(path.c_str(), &animCount);

		if (npc.modelAnim == nullptr && animCount > 0) {
			npc.animCount = 0;
			std::cout << "Failed to load animations for: " << path << "\n";
		}
		else
			npc.animCount = animCount;

		Shared::npcs[npc.name] = npc;
	}
}

void RealEditor::DrawNpc()
{
	for (auto& [name, npc] : Shared::npcs) {
		DrawModelEx(npc.model, npc.position, { 0, 0, 0 }, 0.0f, npc.scale, WHITE);
		DrawBoundingBox(npc.collider, RED);
	}

}

void RealEditor::UpdateNpc()
{
	if (camMode == false) return;//Player Cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

	//Remove npc
	for (auto& [name, npc] : Shared::npcs) {

		RayCollision hit = GetRayCollisionBox(ray, npc.collider);
		if (hit.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selectedNpc = &npc;
			break;
		}
	}

	//Deselect
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		selectedNpc = nullptr;
	}
}

void RealEditor::RemoveNpc()
{
	if (camMode == false) return;//Player Cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

	//Remove npc
	for (auto& [name, npc]: Shared::npcs) {

		RayCollision hit = GetRayCollisionBox(ray, npc.collider);
		if (hit.hit && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Shared::npcs.erase(name);
			break;
		}
	}

}

bool RealEditor::WallCloserThanSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, freeCam);

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

bool RealEditor::CheckCollisionWall(Camera3D* cam, Vector3* nextPos)
{

	for (auto& wall : Shared::walls) {
		if (wall->isDoor && wall->isOpen) continue;

		Vector2 testPosXZ = { nextPos->x, nextPos->z };
		float wallMinY = wall->floor * wall->wallHeight;
		float wallMaxY = wallMinY + wall->wallHeight;

		if (CheckCollisionPointLine(testPosXZ, wall->startPos, wall->endPos, 2) && (playerBottom <= wallMaxY && playerTop >= wallMinY)){
			nextPos->x = cam->position.x;
			nextPos->z = cam->position.z;
			return true;
		}
	}

	return false;
}

bool RealEditor::CheckCollisionSector(Camera3D* cam, Vector3* nextPos)
{

	for(auto& sector: Shared::sectors) {

		//Collision with floor/ceiling(All floor points are on the same y so just use 1 of them)....Same for ceiling
		float floorY = sector.floor * Shared::wallHeight;
		float ceilingY = floorY + Shared::wallHeight - 1.0f;
		
		if (playerTop > floorY && playerBottom < floorY && sector.hasFloor && PointInSector({nextPos->x, nextPos->z},sector.floorPoints))
		{
			nextPos->y = floorY + playerHeight / 2;
			return true;
		}

		if (playerBottom < ceilingY && playerTop > ceilingY && sector.hasCeiling && PointInSector({ nextPos->x, nextPos->z }, sector.ceilingPoints))
		{
			nextPos->y = ceilingY - playerHeight / 2;
			return true;
		}

	}

	return false;
}

bool RealEditor::CheckCollisionRamp(Camera3D* cam, Vector3* nextPos)
{
    const float skin = 0.05f; // same resting-contact margin used for entities

    for (auto& ramp : Shared::ramps) {

        Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
        Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });

        float lowY = (float)ramp.rampHeight * ramp.floor;
        float highY = (float)ramp.rampHeight * ramp.floor + ramp.rampHeight;

        float rampLength = Vector2Distance(ramp.startPos, ramp.endPos);

        Vector3 toPlayer = { nextPos->x - ramp.startPos.x, 0, nextPos->z - ramp.startPos.y };

        float along = toPlayer.x * dir.x + toPlayer.z * dir.y;
        float across = toPlayer.x * perp.x + toPlayer.z * perp.z;

        if (along < 0.0f || along > rampLength) continue;
        if (across < -ramp.halfWidth || across > ramp.halfWidth) continue;

        float t = along / rampLength;
        float rampY = lowY + (highY - lowY) * t; // local surface height at this point

        if (verticalVelocity <= 0.0f)
        {
            // Falling or resting -> only land if we're actually straddling the surface
            if (playerBottom <= rampY && playerTop >= rampY)
            {
                nextPos->y = rampY + playerHeight / 2 + skin;
                return true;
            }
        }
        else
        {
            // Moving upward -> treat the ramp's underside like a ceiling, bump instead of climb
            if (playerTop >= rampY && playerBottom < rampY)
            {
                nextPos->y = rampY - playerHeight / 2 - skin;
                return true;
            }
        }
    }

    return false;
}

bool RealEditor::PointInSector(Vector2 point, const std::vector<Vector2>& poly)
{
	bool inside = false;

	for (int i = 0, j = poly.size() - 1; i < poly.size(); j = i++)
	{
		if (((poly[i].y > point.y) != (poly[j].y > point.y)) &&
			(point.x < (poly[j].x - poly[i].x) *
				(point.y - poly[i].y) /
				(poly[j].y - poly[i].y) + poly[i].x))
		{
			inside = !inside;
		}
	}

	return inside;
}

bool RealEditor::CheckLineRectCollision(Vector2 lineStart, Vector2 lineEnd, Rectangle rect)
{
	Vector2 topLeft = { rect.x, rect.y };
	Vector2 topRight = { rect.x + rect.width, rect.y };
	Vector2 bottomLeft = { rect.x, rect.y + rect.height };
	Vector2 bottomRight = { rect.x + rect.width, rect.y + rect.height };

	// Check the 4 rectangle edges
	if (CheckCollisionLines(lineStart, lineEnd, topLeft, topRight, nullptr))
		return true;

	if (CheckCollisionLines(lineStart, lineEnd, topRight, bottomRight, nullptr))
		return true;

	if (CheckCollisionLines(lineStart, lineEnd, bottomRight, bottomLeft, nullptr))
		return true;

	if (CheckCollisionLines(lineStart, lineEnd, bottomLeft, topLeft, nullptr))
		return true;

	// Optional: if the line is completely inside the rectangle
	if (CheckCollisionPointRec(lineStart, rect) ||
		CheckCollisionPointRec(lineEnd, rect))
		return true;

	return false;
}

bool RealEditor::CheckCollisionPlayerEntityXZ(Camera3D* cam, Vector3* nextPos)
{

	if (cam != &playerCam)
		return false;

	playerCollider.min = {
		nextPos->x - playerRadius,
		nextPos->y - playerHeight / 2,
		nextPos->z - playerRadius
	};

	playerCollider.max = {
		nextPos->x + playerRadius,
		nextPos->y + playerHeight / 2,
		nextPos->z + playerRadius
	};

	for (auto& e : Shared::entities) {
		if (CheckCollisionBoxes(e.collider, playerCollider)) {
			return true;
		}
	}
	return false;
}

bool RealEditor::CheckCollisionPlayerEntityY(Camera3D* cam, Vector3* nextPos)
{
	if (cam != &playerCam)
		return false;

	const float gap = 0.05f; //Small value so that the player is not exactly on or bellow the entity(enables the player to collide smoothly and move)

	playerCollider.min = {
		nextPos->x - playerRadius,
		nextPos->y - playerHeight / 2,
		nextPos->z - playerRadius
	};

	playerCollider.max = {
		nextPos->x + playerRadius,
		nextPos->y + playerHeight / 2,
		nextPos->z + playerRadius
	};

	for (auto& e : Shared::entities) {
		if (CheckCollisionBoxes(e.collider, playerCollider)) {

			float eBottom = e.collider.min.y;
			float eTop = e.collider.max.y;

			if (verticalVelocity <= 0.0f)
			{
				// Land slightly ABOVE the top face
				nextPos->y = eTop + playerHeight / 2 + gap;
				return true;
			}
			else
			{
				//stop slightly BELOW the underside
				nextPos->y = eBottom - playerHeight / 2 - gap;
				return true;
			}
		}
	}
	return false;
}

BoundingBox RealEditor::GetUpdatedBoundingBox(const BoundingBox& original, Vector3 position, Vector3 scale)
{
	BoundingBox box;

	box.min =
	{
		position.x + original.min.x * scale.x,
		position.y + original.min.y * scale.y,
		position.z + original.min.z * scale.z
	};

	box.max =
	{
		position.x + original.max.x * scale.x,
		position.y + original.max.y * scale.y,
		position.z + original.max.z * scale.z
	};

	return box;
}


