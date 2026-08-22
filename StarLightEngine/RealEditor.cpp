#include "RealEditor.h"
#include "raymath.h"
#include "rlgl.h"
#include <iostream>
#include "Cutscene.h"
#include "Audio.h"
#include "imgui/imgui.h"

RealEditor::RealEditor()
{
	InitCam();

}

void RealEditor::InitCam()
{

	//Free Cam
	Shared::free.cam.fovy = 60;
	Shared::free.cam.position = { 1024,50,1024};//1024
	Shared::free.cam.projection = CAMERA_PERSPECTIVE;
	Shared::free.cam.target = { 0 };
	Shared::free.cam.up = { 0.0f, 1.0f, 0.0f };

	Shared::free.forward = Vector3Normalize(Vector3Subtract(Shared::free.cam.target, Shared::free.cam.position));
	Shared::free.yaw = atan2f(Shared::free.forward.z, Shared::free.forward.x);
	Shared::free.pitch = asinf(Shared::free.forward.y);

	//Player Cam
	Shared::player.cam.fovy = 60;
	Shared::player.cam.position = { 1024,50,1024 };
	Shared::player.cam.projection = CAMERA_PERSPECTIVE;
	Shared::player.cam.target = { 0 };
	Shared::player.cam.up = { 0.0f, 1.0f, 0.0f };

	Shared::player.forward = Vector3Normalize(Vector3Subtract(Shared::player.cam.target, Shared::player.cam.position));
	Shared::player.yaw = atan2f(Shared::player.forward.z, Shared::player.forward.x);
	Shared::player.pitch = asinf(Shared::player.forward.y);
}

void RealEditor::Update(Camera3D* cam)
{
	//Update Cursor Lock
	static bool cursorLocked = true;
	if(IsKeyPressed(KEY_TAB))
	{
		cursorLocked = !cursorLocked;
		if (cursorLocked) {
			DisableCursor();
			ImGui::SetWindowFocus(NULL);//Disable the imgui windows when i am in the cursor locked mode.
		}
		else {
			EnableCursor();
		}

	}

	if(!cursorLocked || Shared::onImguiWindow) return;
	
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

	CreateWorldObject();
	UpdateWorldObject();

	CreateNpc();
	UpdateNpc();

	int c = 1; bool loop = true;
	for (auto& [name, npc] : Shared::npcs)
		npc.playAnim(c, loop);

}

void RealEditor::UpdateCam(Camera3D* cam)
{
	float dt = GetFrameTime();
	Vector3 move = { 0 };
	Vector2 mouseDt = GetMouseDelta();

	if (cam == &Shared::free.cam) {
		camMode = true;
		//Camera

		if (freeCamToggle) {
			Shared::free.yaw += -mouseDt.x * (mouseSens * DEG2RAD);
			Shared::free.pitch += -mouseDt.y * (mouseSens * DEG2RAD);
		}
		Shared::free.pitch = Clamp(Shared::free.pitch, -89.0f * DEG2RAD, 89.0f * DEG2RAD);

		Shared::free.forward.x = cosf(Shared::free.pitch) * sinf(Shared::free.yaw);
		Shared::free.forward.y = sinf(Shared::free.pitch);
		Shared::free.forward.z = cosf(Shared::free.pitch) * cosf(Shared::free.yaw);

		Shared::free.forward = Vector3Normalize(Shared::free.forward);

		Vector3 right = Vector3Normalize(Vector3CrossProduct(Shared::free.forward, { 0,1,0 }));
		Vector3 up = Vector3Normalize(Vector3CrossProduct(right, Shared::free.forward));

		if (freeCamToggle) {
			//Movement
			if (IsKeyDown(KEY_W))
				move = Vector3Add(move, Vector3Scale(Shared::free.forward, Shared::free.moveSpeed * dt));
			if (IsKeyDown(KEY_S))
				move = Vector3Add(move, Vector3Scale(Shared::free.forward, -Shared::free.moveSpeed * dt));
			if (IsKeyDown(KEY_D))
				move = Vector3Add(move, Vector3Scale(right, Shared::free.moveSpeed * dt));
			if (IsKeyDown(KEY_A))
				move = Vector3Add(move, Vector3Scale(right, -Shared::free.moveSpeed * dt));
			if (IsKeyDown(KEY_E))
				move = Vector3Add(move, Vector3Scale(up, Shared::free.moveSpeed * dt));
			if (IsKeyDown(KEY_Q))
				move = Vector3Add(move, Vector3Scale(up, -Shared::free.moveSpeed * dt));

		}

		Shared::free.cam.position = Vector3Add(Shared::free.cam.position, move);
		Shared::free.cam.target = Vector3Add(Shared::free.cam.position, Shared::free.forward);
	}
	else if (cam == &Shared::player.cam) {
		camMode = false;
		//Update Position from TileEditor
		if (Shared::setPlayerPos)
		{
			Shared::player.cam.position.x = Shared::playerPos.x;
			Shared::player.cam.position.y = Shared::playerPos.y;
			Shared::player.cam.position.z = Shared::playerPos.z;
			Shared::setPlayerPos = false;
		}

		//Camera
		Shared::player.yaw += -mouseDt.x * (mouseSens * DEG2RAD);
		Shared::player.pitch += -mouseDt.y * (mouseSens * DEG2RAD);

		Shared::player.pitch = Clamp(Shared::player.pitch, -89.0f * DEG2RAD, 89.0f * DEG2RAD);

		Shared::player.forward.x = cosf(Shared::player.pitch) * sinf(Shared::player.yaw);
		Shared::player.forward.y = sinf(Shared::player.pitch);
		Shared::player.forward.z = cosf(Shared::player.pitch) * cosf(Shared::player.yaw);

		Shared::player.forward = Vector3Normalize(Shared::player.forward);

		Vector3 right = Vector3Normalize(Vector3CrossProduct(Shared::player.forward, { 0,1,0 }));
		Vector3 up = Vector3Normalize(Vector3CrossProduct(right, Shared::player.forward));

		//MOVEMENT AND WALKING SFX
		bool walking = IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) || IsKeyDown(KEY_D);
		Audio::Play(Shared::sounds["running"], true, walking);

		if (IsKeyDown(KEY_W))
			move = Vector3Add(move, Vector3Scale(Shared::player.forward, Shared::player.moveSpeed * dt));
		if (IsKeyDown(KEY_S))
			move = Vector3Add(move, Vector3Scale(Shared::player.forward, -Shared::player.moveSpeed * dt));
		if (IsKeyDown(KEY_D))
			move = Vector3Add(move, Vector3Scale(right, Shared::player.moveSpeed * dt));
		if (IsKeyDown(KEY_A))
			move = Vector3Add(move, Vector3Scale(right, -Shared::player.moveSpeed * dt));


		//COLLISION
		Vector3 moveX = { move.x, 0, 0 };
		Vector3 moveY = { 0, move.y, 0 };
		Vector3 moveZ = { 0, 0, move.z };

		Vector3 nextPosX = Vector3Add(Shared::player.cam.position, moveX);
		Vector3 nextPosY = Vector3Add(Shared::player.cam.position, moveY);
		Vector3 nextPosZ = Vector3Add(Shared::player.cam.position, moveZ);
		

		//Check XZ Wall Collision
		if (CheckCollisionWall(cam, &nextPosX))                         
			move.x = 0;                                             
		if (CheckCollisionWall(cam, &nextPosZ))
			move.z = 0;


		//Check collision with entities
		if (CheckCollisionPlayerEntityXZ(&Shared::player.cam, &nextPosX)) {
			move.x = 0;
		}
		if (CheckCollisionPlayerEntityXZ(&Shared::player.cam, &nextPosZ)) {
			move.z = 0;
		}
		

		// Gravity
		if (!Shared::player.isGround)
			Shared::player.verticalVelocity -= Shared::player.gravity * dt;

		// Jump
		if (IsKeyPressed(KEY_SPACE) && Shared::player.isGround)
		{
			Shared::player.verticalVelocity = Shared::player.jump;
			Shared::player.isGround = false;
		}

		// Apply vertical velocity
		move.y = Shared::player.verticalVelocity * dt;

		// Calculate the next Y position AFTER move.y is updated
		nextPosY = Shared::player.cam.position;
		nextPosY.y += move.y;

		UpdatePlayerHeight(&Shared::player.cam, &nextPosY);//Renews the player Bottom and Top values.

		// Check collision
		if (CheckCollisionSector(cam, &nextPosY) || CheckCollisionRamp(cam, &nextPosY))
		{
			Shared::player.verticalVelocity = 0.0f;
			Shared::player.isGround = true;

			move.y = nextPosY.y - Shared::player.cam.position.y;
		}
		else
		{
			Shared::player.isGround = false;
		}

		if (CheckCollisionPlayerEntityY(&Shared::player.cam, &nextPosY)) {
			Shared::player.verticalVelocity = 0;
			move.y = nextPosY.y - Shared::player.cam.position.y;
			Shared::player.isGround = true;
		}

		// Apply updated position and target
		Shared::player.cam.position = Vector3Add(Shared::player.cam.position, move);
		Shared::player.cam.target = Vector3Add(Shared::player.cam.position, Shared::player.forward);
	}

}
void RealEditor::UpdatePlayerHeight(Camera3D* cam, Vector3* nextPos)
{
	Shared::player.bottom = nextPos->y - Shared::player.height / 2;
	Shared::player.top = nextPos->y + Shared::player.height / 2;
}
void RealEditor::UpdateSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);


	float closestFloorDist = FLT_MAX;
	float closestCeilingDist = FLT_MAX;

	for (auto& sector : Shared::sectors)
	{
		// floor
		for (int j = 0; j < sector.indices.size(); j += 3)
		{
			int i0 = sector.indices[j];
			int i1 = sector.indices[j + 1];
			int i2 = sector.indices[j + 2];

			Vector3 p0 = {
				sector.floorPoints[i0].x,
				(float)(sector.floor * Shared::wallHeight),
				sector.floorPoints[i0].y
			};

			Vector3 p1 = {
				sector.floorPoints[i1].x,
				(float)(sector.floor * Shared::wallHeight),
				sector.floorPoints[i1].y
			};

			Vector3 p2 = {
				sector.floorPoints[i2].x,
				(float)(sector.floor * Shared::wallHeight),
				sector.floorPoints[i2].y
			};

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);

			if (col.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				col.distance < closestFloorDist)
			{
				closestFloorDist = col.distance;
				selectedFloor = &sector;
				selectedCeiling = nullptr;
			}
		}
		// ceiling
		for (int j = 0; j < sector.indices.size(); j += 3)
		{
			int i0 = sector.indices[j];
			int i1 = sector.indices[j + 1];
			int i2 = sector.indices[j + 2];

			float y = (float)(sector.floor * Shared::wallHeight)+ Shared::wallHeight - 1.0f;

			Vector3 p0 = { sector.ceilingPoints[i0].x, y, sector.ceilingPoints[i0].y };
			Vector3 p1 = { sector.ceilingPoints[i1].x, y, sector.ceilingPoints[i1].y };
			Vector3 p2 = { sector.ceilingPoints[i2].x, y, sector.ceilingPoints[i2].y };

			RayCollision col = GetRayCollisionTriangle(ray, p0, p1, p2);

			if (col.hit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
				col.distance < closestCeilingDist)
			{
				closestCeilingDist = col.distance;
				selectedCeiling = &sector;
				selectedFloor = nullptr;
			}
		}
	}

	//Deselect
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		selectedCeiling = nullptr;
		selectedFloor = nullptr;
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		if (selectedFloor)
			selectedFloor->floorTextureName = *Shared::selectedTextureName;
		if (selectedCeiling)
			selectedCeiling->ceilingTextureName = *Shared::selectedTextureName;
	}
	
}
void RealEditor::UpdateWall()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

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
		closestWall->textureName = *Shared::selectedTextureName;
	}

}
void RealEditor::UpdateRamp()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

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
		//Change Ramp Height
		if (IsKeyDown(KEY_UP))
			selectedRamp->rampHeight += 1.0f;
		else if (IsKeyDown(KEY_DOWN))
			selectedRamp->rampHeight -= 1.0f;
		//Change texture
		if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
			selectedRamp->textureName = *Shared::selectedTextureName;

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
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

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
			sign = (Vector3DotProduct(Shared::free.forward, perp) < 0.0f) ? 1.0f : -1.0f;
			savedPerp = perp;

			break;
		}
		//Deselect
		else if(!dCol.hit && !IsKeyDown(KEY_C))
			selectedDoorSwitch = nullptr;
	}

	if (selectedDoorSwitch != nullptr)
	{
		if (IsKeyPressed(KEY_R) && selectedDoorSwitch->linkedWall != nullptr )
			selectedDoorSwitch->linkedWall->isOpen = !selectedDoorSwitch->linkedWall->isOpen;

		if (IsKeyDown(KEY_C))
		{
			Vector2 mouseDelta = GetMouseDelta();

			selectedDoorSwitch->pos = Vector3Add(
				selectedDoorSwitch->pos,
				Vector3Scale({ savedDir.x, 0, savedDir.y }, mouseDelta.x * 1.25f * sign)
			);

			selectedDoorSwitch->pos = Vector3Add(
				selectedDoorSwitch->pos,
				Vector3Scale({ 0, 1, 0 }, mouseDelta.y * -1.25f)
			);

		}
		
		if (GetMouseWheelMove() != 0)
		{
			float wheelMove = GetMouseWheelMove();
			selectedDoorSwitch->pos = Vector3Add({ selectedDoorSwitch->pos.x, (selectedDoorSwitch->floor * Shared::wallHeight) + selectedDoorSwitch->pos.y, selectedDoorSwitch->pos.z }, Vector3Scale(savedPerp, wheelMove * sign));

		}

		//Deselect Door Switch
		if(IsKeyReleased(KEY_C))
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
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::player.cam);

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
			sign = (Vector3DotProduct(Shared::free.forward, perp) < 0.0f) ? 1.0f : -1.0f;
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
		rlSetTexture(Shared::textures[d.textureName].id);

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
		rlSetTexture(Shared::textures[wall->textureName].id);

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

	for (auto& sector : Shared::sectors)
	{
		// Floor
		if (sector.hasFloor)
		{
			rlBegin(RL_TRIANGLES);
			rlSetTexture(Shared::textures[sector.floorTextureName].id);
			
			if (selectedFloor == &sector)
				rlColor4ub(255, 0, 0, 150);
			else
				rlColor4ub(255, 255, 255, 255);

			for (int i = 0; i < sector.indices.size(); i += 3)
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
			rlSetTexture(Shared::textures[sector.ceilingTextureName].id);
			
			if (selectedCeiling == &sector)
				rlColor4ub(255, 0, 0, 150);
			else
				rlColor4ub(255, 255, 255, 255);

			for (int i = 0; i  < sector.indices.size(); i += 3)
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

		rlBegin(RL_QUADS);
		rlSetTexture(Shared::textures[ramp.textureName].id);

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
	}
}

void RealEditor::Draw()
{
	//Disable back face culling
	rlDisableBackfaceCulling();

	//Draw Walls
	DrawWall();

	DrawRamp();

	DrawDoorSwitch();

	DrawWorldObject();

	DrawNpc();
	

	DrawSector();
}

void RealEditor::UpdateWorldObject()
{
	if (camMode == false || Shared::isCutscenePlaying) return;//Player Cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

	//select world objects and stop camera from moving when an entity is selected.
	for (auto& [id, worldObject] : Shared::worldObjects) {
		RayCollision hit = GetRayCollisionBox(ray, worldObject.collider);
		if (hit.hit && IsKeyPressed(KEY_C)) {
			Shared::selectedWorldObject = &worldObject;
			freeCamToggle = false;
		}
		if (hit.hit && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			//Delet entity and its cutscene tracks.
			if (Shared::cutscenes.worldObjectCutscene.contains(id)) {
				Shared::cutscenes.worldObjectCutscene.erase(id);
			}
			Shared::worldObjects.erase(id);
			Shared::selectedWorldObject = nullptr;
			freeCamToggle = true;
			break;
		}
	}

	//If an entity is selected
	if (Shared::selectedWorldObject != nullptr) {
		//Move the entity and its collider
		Vector3 perp = Vector3CrossProduct(Shared::free.forward, { 0,1,0 });
		Vector3 delta =Vector3Normalize(Vector3Add(Vector3Scale(perp, GetMouseDelta().x), Vector3Scale(Shared::free.forward, -GetMouseDelta().y)));
		Vector3 move = Vector3Scale(delta, 4.0f);
		
		if (IsKeyPressed(KEY_UP))
			move.y = 4.0f;
		else if (IsKeyPressed(KEY_DOWN)) 
			move.y = -4.0f;
		else
			move.y = 0.0f;

		//Updating Y
		Shared::selectedWorldObject->position = Vector3Add(Shared::selectedWorldObject->position, {0,move.y, 0});

		//Updating XZ and check Wall collisons

		Shared::selectedWorldObject->position = Vector3Add(Shared::selectedWorldObject->position, { move.x, 0, move.z});

		//Convert the collider into 2d XZ Plane to check if it collides with a wall.
		Rectangle rect;
		rect.x = Shared::selectedWorldObject->collider.min.x;
		rect.y = Shared::selectedWorldObject->collider.min.z;

		rect.width = Shared::selectedWorldObject->collider.max.x - Shared::selectedWorldObject->collider.min.x;
		rect.height = Shared::selectedWorldObject->collider.max.z - Shared::selectedWorldObject->collider.min.z;

		for (auto& wall : Shared::walls) {
			if (CheckLineRectCollision(wall->startPos, wall->endPos, rect))
			{
				Shared::selectedWorldObject->position = Vector3Subtract(Shared::selectedWorldObject->position, { move.x, 0, move.z });
				
			}
		}


		//Change Scale while pressing arrow keys
		if (IsKeyPressed(KEY_RIGHT)) {
			Shared::selectedWorldObject->scale = Vector3Add(Shared::selectedWorldObject->scale, {2,2,2});
		}
		else if (IsKeyPressed(KEY_LEFT)) {
			Shared::selectedWorldObject->scale = Vector3Subtract(Shared::selectedWorldObject->scale, {2,2,2});
		}
		Shared::selectedWorldObject->scale = Vector3Clamp(Shared::selectedWorldObject->scale, { 1, 1, 1 }, { 200,200, 200 });
		
		//Change the rotation of the entity while pressing Q and E
		if(IsKeyPressed(KEY_Q))//Rotate left
			Shared::selectedWorldObject->rotationAngle -= 15.0f;
		else if (IsKeyPressed(KEY_E))//Rotate right
			Shared::selectedWorldObject->rotationAngle += 15.0f;

		//Update the rotation Axis
		Shared::selectedWorldObject->rotationAxis = { 0, 1, 0 };


		
		//Update the collider
		Shared::selectedWorldObject->collider = Shared::GetUpdatedBoundingBox(Shared::selectedWorldObject->originalCollider, Shared::selectedWorldObject->position, Shared::selectedWorldObject->scale, Shared::selectedWorldObject->rotationAxis, Shared::selectedWorldObject->rotationAngle);


	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		Shared::selectedWorldObject = nullptr;
		freeCamToggle = true;
	}

	//Deselect entity
	if (Shared::selectedWorldObject != nullptr && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		Shared::selectedWorldObject = nullptr;
		freeCamToggle = true;

	}

}

void RealEditor::CreateWorldObject()
{
	if (Shared::selectedModelName == nullptr || selectedFloor == nullptr || Shared::npcMode) return;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		WorldObject worldObject;
		worldObject.id = WorldObject::currentId;
		WorldObject::currentId++;
		
		worldObject.modelName = *Shared::selectedModelName;

		Vector3 direction = Vector3Normalize(Vector3Subtract(Shared::free.cam.target, Shared::free.cam.position));
		Vector3 spawnPos = Vector3Add(Shared::free.cam.position, Vector3Scale(direction, 50.0f)); // Spawn 5 units in front of the camera

		worldObject.position = { spawnPos.x, (float)selectedFloor->floor * Shared::wallHeight, spawnPos.z };

		BoundingBox box = GetModelBoundingBox(Shared::models[worldObject.modelName]);
		worldObject.originalCollider = box;

		box.min.x *= worldObject.scale.x;
		box.min.y *= worldObject.scale.y;
		box.min.z *= worldObject.scale.z;

		box.max.x *= worldObject.scale.x;
		box.max.y *= worldObject.scale.y;
		box.max.z *= worldObject.scale.z;


		box.min = Vector3Add(box.min, worldObject.position);
		box.max = Vector3Add(box.max, worldObject.position);

		worldObject.collider = box;

		Shared::worldObjects[worldObject.id] = worldObject;
		Shared::selectedModelName = nullptr;
	}
	
}

void RealEditor::DrawWorldObject()
{
	for (auto& [id, worldObject] : Shared::worldObjects) {
		
		DrawModelEx(Shared::models[worldObject.modelName], worldObject.position, worldObject.rotationAxis, worldObject.rotationAngle, worldObject.scale, WHITE);
		DrawBoundingBox(worldObject.collider, RED);
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


		npc.model = Shared::models[npc.modelName];
		npc.scale = { 0.5, 0.5, 0.5 };

		//Position
		Vector3 direction = Vector3Normalize(Vector3Subtract(Shared::free.cam.target, Shared::free.cam.position));
		Vector3 spawnPos = Vector3Add(Shared::free.cam.position, Vector3Scale(direction, 50.0f)); // Spawn 5 units in front of the camera
		npc.position = { spawnPos.x, (float)selectedFloor->floor * Shared::wallHeight, spawnPos.z };

		//Collider
		BoundingBox box = GetModelBoundingBox(Shared::models[npc.modelName]);
		npc.originalCollider = box;

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
	if (camMode == false || Shared::isCutscenePlaying) return;//Player Cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

	//select npc and stop camera from moving when an npc is selected.
	for (auto& [name, npc] : Shared::npcs) {
		RayCollision hit = GetRayCollisionBox(ray, npc.collider);
		if (hit.hit && IsKeyPressed(KEY_C)) {
			Shared::selectedNpc = &npc;
			freeCamToggle = false;
		}

		//Remove
		if (hit.hit && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			//Delete NPC nad its tracks
			if (Shared::cutscenes.npcCutscene.contains(name)) {
				Shared::cutscenes.npcCutscene.erase(name);
			}
			Shared::npcs.erase(name);
			Shared::selectedNpc = nullptr;

			break;
		}
	}

	//Update Npc translations
	if (Shared::selectedNpc != nullptr) {
		//Move the entity and its collider
		Vector3 perp = Vector3CrossProduct(Shared::free.forward, { 0,1,0 });
		Vector3 delta = Vector3Normalize(Vector3Add(Vector3Scale(perp, GetMouseDelta().x), Vector3Scale(Shared::free.forward, -GetMouseDelta().y)));
		Vector3 move = Vector3Scale(delta, 8.0f);

		if (IsKeyPressed(KEY_UP))
			move.y = 4.0f;
		else if (IsKeyPressed(KEY_DOWN))
			move.y = -4.0f;
		else
			move.y = 0.0f;

		//Updating Y
		Shared::selectedNpc->position = Vector3Add(Shared::selectedNpc->position, { 0,move.y, 0 });

		//Updating XZ and check Wall collisons

		Shared::selectedNpc->position = Vector3Add(Shared::selectedNpc->position, { move.x, 0, move.z });

		//Convert the collider into 2d XZ Plane to check if it collides with a wall.
		Rectangle rect;
		rect.x = Shared::selectedNpc->collider.min.x;
		rect.y = Shared::selectedNpc->collider.min.z;

		rect.width = Shared::selectedNpc->collider.max.x - Shared::selectedNpc->collider.min.x;
		rect.height = Shared::selectedNpc->collider.max.z - Shared::selectedNpc->collider.min.z;

		for (auto& wall : Shared::walls) {
			if (CheckLineRectCollision(wall->startPos, wall->endPos, rect))
			{
				Shared::selectedNpc->position = Vector3Subtract(Shared::selectedNpc->position, { move.x, 0, move.z });
				
			}
		}


		//Change Scale while pressing arrow keys
		if (IsKeyPressed(KEY_RIGHT)) {
			Shared::selectedNpc->scale = Vector3Add(Shared::selectedNpc->scale, { 2,2,2 });
		}
		else if (IsKeyPressed(KEY_LEFT)) {
			Shared::selectedNpc->scale = Vector3Subtract(Shared::selectedNpc->scale, { 2,2,2 });
		}
		Shared::selectedNpc->scale = Vector3Clamp(Shared::selectedNpc->scale, { 0.5, 0.5, 0.5 }, { 200,200, 200 });


		Shared::selectedNpc->collider = Shared::GetUpdatedBoundingBox(Shared::selectedNpc->originalCollider, Shared::selectedNpc->position, Shared::selectedNpc->scale, Shared::selectedNpc->rotationAxis, Shared::selectedNpc->rotationAngle);


	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		Shared::selectedNpc = nullptr;
		freeCamToggle = true;
	}
}

bool RealEditor::WallCloserThanSector()
{
	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);

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

		if (CheckCollisionPointLine(testPosXZ, wall->startPos, wall->endPos, 2) && (Shared::player.bottom <= wallMaxY && Shared::player.top >= wallMinY)){
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
		
		if (Shared::player.top > floorY && Shared::player.bottom < floorY && sector.hasFloor && PointInSector({nextPos->x, nextPos->z},sector.floorPoints))
		{
			nextPos->y = floorY + Shared::player.height / 2;
			return true;
		}

		if (Shared::player.bottom < ceilingY && Shared::player.top > ceilingY && sector.hasCeiling && PointInSector({ nextPos->x, nextPos->z }, sector.ceilingPoints))
		{
			nextPos->y = ceilingY - Shared::player.height / 2;
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

		// Falling or resting -> only land if we're actually straddling the surface
		if (Shared::player.bottom <= rampY && Shared::player.top > rampY)
		{
			nextPos->y = rampY + Shared::player.height / 2 + skin;
			return true;
		}
		// Moving upward -> treat the ramp's underside like a ceiling, bump instead of climb
		else if (Shared::player.top >= rampY && Shared::player.bottom < rampY)
		{
			nextPos->y = rampY - Shared::player.height / 2 - skin;
			return true;
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

	if (cam != &Shared::player.cam)
		return false;

	Shared::player.collider.min = {
		nextPos->x - Shared::player.radius,
		nextPos->y - Shared::player.height / 2,
		nextPos->z - Shared::player.radius
	};

	Shared::player.collider.max = {
		nextPos->x + Shared::player.radius,
		nextPos->y + Shared::player.height / 2,
		nextPos->z + Shared::player.radius
	};

	for (auto& [id, entity] : Shared::worldObjects) {
		if (CheckCollisionBoxes(entity.collider, Shared::player.collider)) {
			return true;
		}
	}
	return false;
}

bool RealEditor::CheckCollisionPlayerEntityY(Camera3D* cam, Vector3* nextPos)
{
	if (cam != &Shared::player.cam)
		return false;

	const float gap = 0.05f; //Small value so that the player is not exactly on or bellow the entity(enables the player to collide smoothly and move)

	Shared::player.collider.min = {
		nextPos->x - Shared::player.radius,
		nextPos->y - Shared::player.height / 2,
		nextPos->z - Shared::player.radius
	};

	Shared::player.collider.max = {
		nextPos->x + Shared::player.radius,
		nextPos->y + Shared::player.height / 2,
		nextPos->z + Shared::player.radius
	};

	for (auto& [id, e] : Shared::worldObjects) {
		if (CheckCollisionBoxes(e.collider, Shared::player.collider)) {

			float eBottom = e.collider.min.y;
			float eTop = e.collider.max.y;

			if (Shared::player.verticalVelocity <= 0.0f)
			{
				// Land slightly ABOVE the top face
				nextPos->y = eTop + Shared::player.height / 2 + gap;
				return true;
			}
			else
			{
				//stop slightly BELOW the underside
				nextPos->y = eBottom - Shared::player.height / 2 - gap;
				return true;
			}
		}
	}
	return false;
}
