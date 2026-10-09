#include "RealEditor.h"
#include "raymath.h"
#include "rlgl.h"
#include "raylib.h"
#include <iostream>
#include "Cutscene.h"
#include "Audio.h"
#include "imgui/imgui.h"
#include "LightSystem.h"
#include "DialougeSystem.h"

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

	Shared::freeRay = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);
	Shared::playerRay = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::player.cam);


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

	//World Objects
	CreateWorldObject();
	UpdateWorldObject();

	//NPC
	CreateNpc();
	UpdateNpc();

	int c = 1; bool loop = true;
	for (auto& [name, npc] : Shared::npcs)
		npc.playAnim(c, loop);

	//Decals
	if (IsKeyPressed(KEY_LEFT_CONTROL))
		decalMode = true;
	else if (IsKeyReleased(KEY_LEFT_CONTROL))
		decalMode = false;
	CreateDecal();
	UpdateDecal();

	//Light
	LightSystem::Update(camMode, freeCamToggle);

	//Trigger
	UpdateTrigger();
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

			RayCollision col = GetRayCollisionTriangle(Shared::freeRay, p0, p1, p2);

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

			RayCollision col = GetRayCollisionTriangle(Shared::freeRay, p0, p1, p2);

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

	if (decalMode || camMode == false) return;//Player Cam Mode.
	
	//Get the closest wall to the ray.
	Wall* closestWall = GetRayWall(Shared::freeRay);

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
	if (camMode == false) return;//Player Cam Mode.


	for (auto& ramp: Shared::ramps) {

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector3 perp = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });

		float lowY = (float)ramp.rampHeight * ramp.floor;
		float highY = (float)ramp.rampHeight * ramp.floor + ramp.rampHeight;

		Vector3 startLeft = Vector3Subtract({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 startRight = Vector3Add({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endLeft = Vector3Subtract({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));
		Vector3 endRight = Vector3Add({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(perp, ramp.halfWidth));

		RayCollision rampCol = GetRayCollisionQuad(Shared::freeRay, startLeft, endLeft, endRight, startRight);

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
	if (camMode == false) return;//Player Cam Mode.

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
	Ray ray;
	if(camMode)
		ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);
	else
		ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::player.cam);

	static Vector2 savedDir = { 0 };
	static Vector3 savedNormal = { 0 };
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
		Vector3 normal = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });
		if (Vector3DotProduct(ray.direction, normal) > 0.0f)
			normal = Vector3Scale(normal, -1.0f);

		Vector3 bottomLeft = { closestWall->startPos.x, (closestWall->floor * Shared::wallHeight), closestWall->startPos.y };
		Vector3 bottomRight = { closestWall->endPos.x, (closestWall->floor * Shared::wallHeight), closestWall->endPos.y };
		Vector3 topLeft = { closestWall->startPos.x, (closestWall->floor * Shared::wallHeight) + Shared::wallHeight, closestWall->startPos.y };
		Vector3 topRight = { closestWall->endPos.x, (closestWall->floor * Shared::wallHeight) + Shared::wallHeight, closestWall->endPos.y };

		RayCollision dCol = GetRayCollisionQuad(ray, bottomLeft, topLeft, topRight, bottomRight);

		//Select
		if (dCol.hit && (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_R)))
		{
			selectedDoorSwitch = &d;
			savedDir = dir;
			sign = (Vector3DotProduct(Shared::free.forward, normal) < 0.0f) ? -1.0f : 1.0f;
			savedNormal = normal;

			break;
		}
		
	}

	//Deselect
	if(IsKeyReleased(KEY_C))
		selectedDoorSwitch = nullptr;

	if (selectedDoorSwitch != nullptr)
	{
		if (IsKeyPressed(KEY_R) && selectedDoorSwitch->linkedWall != nullptr )
			selectedDoorSwitch->linkedWall->isOpen = !selectedDoorSwitch->linkedWall->isOpen;

		if (camMode) {
			if (IsKeyDown(KEY_C))
			{
				Vector2 mouseDelta = GetMouseDelta();

				selectedDoorSwitch->pos = Vector3Add(selectedDoorSwitch->pos, Vector3Scale({ savedDir.x, 0, savedDir.y }, mouseDelta.x * 1.25f * sign));
				selectedDoorSwitch->pos = Vector3Add(selectedDoorSwitch->pos, Vector3Scale({ 0, 1, 0 }, mouseDelta.y * -1.25f));

			}

			if (GetMouseWheelMove() != 0)
			{
				float wheelMove = GetMouseWheelMove();
				selectedDoorSwitch->pos = Vector3Add({ selectedDoorSwitch->pos.x, (selectedDoorSwitch->floor * Shared::wallHeight) + selectedDoorSwitch->pos.y, selectedDoorSwitch->pos.z }, Vector3Scale(savedNormal, wheelMove * sign));

			}
		}

	}

	float speed = 100.0f * GetFrameTime();
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
			
			float dist = Vector2Distance({ d.pos.x, d.pos.z }, { (wall->startPos.x + wall->endPos.x) / 2, (wall->startPos.y + wall->endPos.y) / 2 });
			if(dist < clostseDist) {
				clostseDist = dist;
				closestWall = wall;
			}
		}

		Vector2 dir = Vector2Normalize(Vector2Subtract(closestWall->endPos, closestWall->startPos));
		Vector3 normal = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });
		
		Vector3 bottomLeft = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ dir.x, 0, dir.y }, -16));
		Vector3 bottomRight = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ dir.x, 0, dir.y }, 16));
		Vector3 topLeft = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ dir.x, 0, dir.y }, -16));
		Vector3 topRight = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ dir.x, 0, dir.y }, 16));

		float tileX = 1.0f;
		float tileY = 1.0f;

		rlBegin(RL_QUADS);
		rlNormal3f(-normal.x, -normal.y, -normal.z);
		rlSetTexture(Shared::textures[d.textureName].id);

		rlColor4ub(255, 255, 255, 255);

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(topRight.x, topRight.y, topRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

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

		Vector3 dir = Vector3Normalize(Vector3Subtract({ wall->endPos.x, (wall->wallHeight * wall->floor), wall->endPos.y }, { wall->startPos.x, (wall->wallHeight * wall->floor), wall->startPos.y }));
		Vector3 normal = Vector3CrossProduct(dir, {0,1,0});
		
		rlBegin(RL_QUADS);
		rlNormal3f(normal.x, normal.y, normal.z);
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
			rlDrawRenderBatchActive();
			rlBegin(RL_TRIANGLES);
			rlNormal3f(0, -1, 0);
			rlSetTexture(Shared::textures[sector.floorTextureName].id);
			
			if (selectedFloor == &sector)
				rlColor4ub(255, 0, 0, 150);
			else
				rlColor4ub(255, 255, 255, 255);

			for (int i = 0; i < sector.indices.size(); i += 3)
			{
				const Vector2& p0 = sector.floorPoints[sector.indices[i]];
				const Vector2& p1 = sector.floorPoints[sector.indices[i + 1]];
				const Vector2& p2 = sector.floorPoints[sector.indices[i + 2]];

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
			rlDrawRenderBatchActive();
			rlBegin(RL_TRIANGLES);
			rlNormal3f(0, 1, 0);
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
		Vector3 normal = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });

		float lowY = (float)ramp.rampHeight * ramp.floor;
		float highY = (float)ramp.rampHeight * ramp.floor + ramp.rampHeight; 

		Vector3 startLeft = Vector3Subtract({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(normal, ramp.halfWidth));
		Vector3 startRight = Vector3Add({ ramp.startPos.x, lowY,  ramp.startPos.y }, Vector3Scale(normal, ramp.halfWidth));
		Vector3 endLeft = Vector3Subtract({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(normal, ramp.halfWidth));
		Vector3 endRight = Vector3Add({ ramp.endPos.x,   highY, ramp.endPos.y }, Vector3Scale(normal, ramp.halfWidth));

		float wallLength = Vector2Distance(ramp.startPos, ramp.endPos);
		float tileX = 12;
		float tileY = 12;

		rlBegin(RL_QUADS);
		rlNormal3f(normal.x, normal.y, normal.z);
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
	rlDisableBackfaceCulling();

	if (Shared::lights.empty()) {
		//No lights - draw once, unlit, so the scene isn't blank
		DrawWall();
		DrawRamp();
		DrawDoorSwitch();
		DrawSector();
		DrawWorldObject(LightSystem::pointShader.shader);
		DrawNpc(LightSystem::pointShader.shader);
		DrawDecals();
	}
	else {
		for (size_t i = 0; i < Shared::lights.size(); i++) {
			Light& light = Shared::lights[i];

			if(!light.enabled)
				continue;

			LightShader* activeLightShader = &LightSystem::pointShader;//Default shader to use
			switch (light.type) {
			case POINT_LIGHT:       activeLightShader = &LightSystem::pointShader; break;
			case SPOT_LIGHT:        activeLightShader = &LightSystem::spotShader; break;
			case DIRECTIONAL_LIGHT: activeLightShader = &LightSystem::directionalShader; break;
			}

			LightSystem::UpdateShaderValues(*activeLightShader, light);

			if (i > 0)
				BeginBlendMode(BLEND_ADDITIVE); // stack this light's contribution on top of previous passes

			BeginShaderMode(activeLightShader->shader);

			DrawWall();
			DrawRamp();
			DrawDoorSwitch();
			DrawSector();
			DrawWorldObject(activeLightShader->shader);
			DrawNpc(activeLightShader->shader);
			DrawDecals();

			EndShaderMode();

			if (i > 0)
				EndBlendMode();
		}
	}

	//----Draw things without the light shader on it---

	//Lights
	LightSystem::DrawLightEX(camMode);
	DrawTrigger();
	DrawRay(Shared::freeRay, RED);
}

void RealEditor::UpdateWorldObject()
{
	if (camMode == false || Shared::isCutscenePlaying) return;//Player Cam

	//select world objects and stop camera from moving when an entity is selected.
	for (auto it = Shared::worldObjects.begin(); it != Shared::worldObjects.end(); ) {

		// 'it' points to the current element in worldObjects.
		// We dereference it (*) to get the actual [id, worldObject] pair.
		auto& [id, worldObject] = *it;

		if (worldObject.collisionBoxes.empty() || worldObject.collisionType == NONE) {
			++it; //Move iterator to the next worldObject.
			continue;
		}

		bool erased = false;

		for (auto& box : worldObject.collisionBoxes) {
			RayCollision hit = GetRayCollisionBox(Shared::freeRay, box);


			if (hit.hit) {
				//Select entity and stop camera from moving.
				if (IsKeyPressed(KEY_C)) {
					Shared::selectedWorldObject = &worldObject;
					freeCamToggle = false;
				}

				//Delete entity and its cutscene tracks.
				if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

					//Delete its Cutscenes
					if (Shared::cutscenes.worldObjectCutscene.contains(id)) {
						Shared::cutscenes.worldObjectCutscene.erase(id);
					}

					//Erase the current worldObject.
					//erase(it) deletes the current object AND returns
					//an iterator pointing to the next worldObject.
					it = Shared::worldObjects.erase(it);

					Shared::selectedWorldObject = nullptr;
					freeCamToggle = true;

					erased = true;

					break;
				}

				break;
			}
		}

		//If we didn't erase the current worldObject,
		//we need to manually move to the next one.
		//
		//If we DID erase it, 'it' is already pointing to the next object,
		//so we must NOT increment it again.
		if (!erased) {
			++it;
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

		//----Updating Y-----
		Shared::selectedWorldObject->position = Vector3Add(Shared::selectedWorldObject->position, {0,move.y, 0});

		
		//----Updating XZ and check Wall collisons------
		//Convert the collider into 2d XZ Plane to check if it collides with a wall.
		static bool collided = false;
		for (auto& box : Shared::selectedWorldObject->collisionBoxes) {
			Rectangle rect;
			rect.x = box.min.x + move.x;
			rect.y = box.min.z + move.z;

			rect.width = box.max.x - box.min.x;
			rect.height = box.max.z - box.min.z;

			for (auto& wall : Shared::walls) {
				if (CheckLineRectCollision(wall->startPos, wall->endPos, rect))
				{
					collided = true;
					break;
				}
			}

			if (collided == true)
				break;
		}

		if(!collided)
			Shared::selectedWorldObject->position = Vector3Add(Shared::selectedWorldObject->position, { move.x, 0, move.z });
		else {
			Shared::selectedWorldObject->position = Vector3Subtract(Shared::selectedWorldObject->position, { move.x, 0, move.z });
			collided = false;
		}

		//----SCALE------

		//Change Scale while pressing arrow keys
		if (IsKeyPressed(KEY_RIGHT)) {
			Shared::selectedWorldObject->scale = Vector3Add(Shared::selectedWorldObject->scale, {2,2,2});
		}
		else if (IsKeyPressed(KEY_LEFT)) {
			Shared::selectedWorldObject->scale = Vector3Subtract(Shared::selectedWorldObject->scale, {2,2,2});
		}
		Shared::selectedWorldObject->scale = Vector3Clamp(Shared::selectedWorldObject->scale, { 1, 1, 1 }, { 200,200, 200 });
		
		//----ROTATION------

		//Change the rotation of the entity while pressing Q and E
		if(IsKeyPressed(KEY_Q))//Rotate left
			Shared::selectedWorldObject->rotationAngle -= 15.0f;
		else if (IsKeyPressed(KEY_E))//Rotate right
			Shared::selectedWorldObject->rotationAngle += 15.0f;

		//Update the collider (always rebuilt from the local-space boxes)
		for (int i = 0; i < Shared::selectedWorldObject->collisionBoxes.size(); i++)
			Shared::selectedWorldObject->collisionBoxes[i] = Shared::GetUpdatedBoundingBox(Shared::selectedWorldObject->originalBoxes[i], Shared::selectedWorldObject->position, Shared::selectedWorldObject->scale, Shared::selectedWorldObject->rotationAxis, Shared::selectedWorldObject->rotationAngle);

	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		Shared::selectedWorldObject = nullptr;
		freeCamToggle = true;
	}

}

void RealEditor::CreateWorldObject()
{
	if (Shared::selectedModelName == nullptr || selectedFloor == nullptr || Shared::npcMode) return;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		WorldObject worldObject;

		//ID
		worldObject.id = WorldObject::currentId;
		WorldObject::currentId++;
		
		//Name
		worldObject.modelName = *Shared::selectedModelName;

		Vector3 direction = Vector3Normalize(Vector3Subtract(Shared::free.cam.target, Shared::free.cam.position));
		Vector3 spawnPos = Vector3Add(Shared::free.cam.position, Vector3Scale(direction, 50.0f)); // Spawn 5 units in front of the camera

		//Position
		worldObject.position = { spawnPos.x, (float)selectedFloor->floor * Shared::wallHeight, spawnPos.z };

		//CollisionType
		worldObject.collisionType = Shared::selectedCollisionType;

		//Collision Box
		if (Shared::selectedCollisionType == CollisionType::MESH) {

			Model& model = Shared::models[worldObject.modelName];

			for (int i = 0; i < model.meshCount; i++) {
				BoundingBox box = GetMeshBoundingBox(model.meshes[i]);
				worldObject.originalBoxes.push_back(box);//Local Coordinates
				worldObject.collisionBoxes.push_back(Shared::GetUpdatedBoundingBox(box, worldObject.position, worldObject.scale, worldObject.rotationAxis, worldObject.rotationAngle));
			}
		}
		else if (Shared::selectedCollisionType == CollisionType::BOX) {
			BoundingBox box = GetModelBoundingBox(Shared::models[worldObject.modelName]);
			worldObject.originalBoxes.push_back(box);
			worldObject.collisionBoxes.push_back(Shared::GetUpdatedBoundingBox(box, worldObject.position, worldObject.scale, worldObject.rotationAxis, worldObject.rotationAngle));
		}

		Shared::worldObjects[worldObject.id] = worldObject;
		Shared::selectedModelName = nullptr;
	}
	
}

void RealEditor::DrawWorldObject(Shader& activeShader)
{
	for (auto& [id, worldObject] : Shared::worldObjects) {

		Model& model = Shared::models[worldObject.modelName];
		for (int i = 0; i < model.materialCount; i++)
			model.materials[i].shader = activeShader;

		DrawModelEx(model, worldObject.position, worldObject.rotationAxis, worldObject.rotationAngle, worldObject.scale, WHITE);
		for(auto& box: worldObject.collisionBoxes) {
			DrawBoundingBox(box, RED);
		}
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


		//Collider/Type
		npc.collisionType = CollisionType::BOX;
		BoundingBox box = GetModelBoundingBox(Shared::models[npc.modelName]);

		//Local Original Coordinates(Original Collision)
		npc.originalBoxes.push_back(box);

		box.min = Vector3Multiply(box.min, npc.scale);
		box.max = Vector3Multiply(box.max, npc.scale);

		box.min = Vector3Add(box.min, npc.position);
		box.max = Vector3Add(box.max, npc.position);

		//World Coordinates(Nerw Collision)
		npc.collisionBoxes.push_back(box);

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

void RealEditor::DrawNpc(Shader activeShader)
{
	for (auto& [name, npc] : Shared::npcs) {
		Model& model = Shared::models[npc.modelName];
		for (int i = 0; i < model.materialCount; i++)
			model.materials[i].shader = activeShader;

		DrawModelEx(model, npc.position, npc.rotationAxis, npc.rotationAngle, npc.scale, WHITE);
		for(auto& box: npc.collisionBoxes) {
			DrawBoundingBox(box, RED);
		}
	}

}

void RealEditor::UpdateNpc()
{
	if (camMode == false || Shared::isCutscenePlaying) return;//Player Cam

	//select npc and stop camera from moving when an npc is selected.
	for (auto& [name, npc] : Shared::npcs) {
		RayCollision hit = GetRayCollisionBox(Shared::freeRay, npc.collisionBoxes[0]);
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
		rect.x = Shared::selectedNpc->collisionBoxes[0].min.x;
		rect.y = Shared::selectedNpc->collisionBoxes[0].min.z;

		rect.width = Shared::selectedNpc->collisionBoxes[0].max.x - Shared::selectedNpc->collisionBoxes[0].min.x;
		rect.height = Shared::selectedNpc->collisionBoxes[0].max.z - Shared::selectedNpc->collisionBoxes[0].min.z;

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

		//Change the rotation of the Npc while pressing Q and E
		if (IsKeyPressed(KEY_Q))//Rotate left
			Shared::selectedNpc->rotationAngle -= 15.0f;
		else if (IsKeyPressed(KEY_E))//Rotate right
			Shared::selectedNpc->rotationAngle += 15.0f;

		//Update the rotation Axis
		Shared::selectedNpc->rotationAxis = Shared::rotationAxis;

		//Update the collider
		Shared::selectedNpc->collisionBoxes[0] = Shared::GetUpdatedBoundingBox(Shared::selectedNpc->originalBoxes[0], Shared::selectedNpc->position, Shared::selectedNpc->scale, Shared::selectedNpc->rotationAxis, Shared::selectedNpc->rotationAngle);

	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		Shared::selectedNpc = nullptr;
		freeCamToggle = true;
	}
}

void RealEditor::CreateDecal()
{
	if (camMode == false || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;//Player Cam Mode.

	if (decalMode) {
		Wall* closestWall = GetRayWall(Shared::freeRay);
		if(!closestWall)
			return;

		Vector2 dir = Vector2Normalize(Vector2Subtract(closestWall->endPos, closestWall->startPos));
		Vector3 normal = Vector3CrossProduct({ dir.x, 0, dir.y }, { 0, 1, 0 });
		if (Vector3DotProduct(Shared::freeRay.direction, normal) > 0.0f)
			normal = Vector3Scale(normal, -1.0f);

		Vector3 bottomLeft = { closestWall->startPos.x, (closestWall->floor * Shared::wallHeight), closestWall->startPos.y };
		Vector3 bottomRight = { closestWall->endPos.x, (closestWall->floor * Shared::wallHeight), closestWall->endPos.y };
		Vector3 topLeft = { closestWall->startPos.x, (closestWall->floor * Shared::wallHeight) + Shared::wallHeight, closestWall->startPos.y };
		Vector3 topRight = { closestWall->endPos.x, (closestWall->floor * Shared::wallHeight) + Shared::wallHeight, closestWall->endPos.y };

		RayCollision dCol = GetRayCollisionQuad(Shared::freeRay, bottomLeft, topLeft, topRight, bottomRight);

		if (dCol.hit) {
			Decal decal;

			decal.pos = Vector3Add(dCol.point, Vector3Scale(normal, 2.f));
			decal.textureName = *Shared::selectedTextureName;
			decal.floor = closestWall->floor;
			decal.wallDir = dir;
			Shared::decals.push_back(decal);
		}
	}

}

void RealEditor::UpdateDecal()
{
	if (camMode == false) return;//Player Cam Mode.

	int index = -1;

	for (int i = 0; i < Shared::decals.size(); i++) {
		auto& d = Shared::decals[i];


		Vector3 bottomLeft = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, -16));
		Vector3 bottomRight = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, 16));
		Vector3 topLeft = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, -16));
		Vector3 topRight = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, 16));

		RayCollision dCol = GetRayCollisionQuad(Shared::freeRay, bottomLeft, topLeft, topRight, bottomRight);
		
		//Select
		if (dCol.hit)
		{
			Shared::selectedDecal = &d;
			index = i;
			break;
		}

	}


	if (Shared::selectedDecal) {

		//Move the decal
		if (IsKeyDown(KEY_C)) {
			Vector2 mouseDelta = GetMouseDelta();

			Vector3 normal = Vector3CrossProduct({ Shared::selectedDecal->wallDir.x, 0, Shared::selectedDecal->wallDir.y }, { 0, 1, 0 });
			float sign = (Vector3DotProduct(Shared::free.forward, normal) < 0.0f) ? 1.0f : -1.0f;


			Shared::selectedDecal->pos = Vector3Add(Shared::selectedDecal->pos, Vector3Scale({ Shared::selectedDecal->wallDir.x, 0, Shared::selectedDecal->wallDir.y }, mouseDelta.x * 1.25f * sign));
			Shared::selectedDecal->pos = Vector3Add(Shared::selectedDecal->pos, Vector3Scale({ 0, 1, 0 }, mouseDelta.y * -1.25f));
		}

		//Delete Decal
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			Shared::selectedDecal = nullptr;
			Shared::decals.erase(Shared::decals.begin() + index);
			index = -1;
		}
	}



}

void RealEditor::DrawDecals()
{
	for (auto& d : Shared::decals) {

		Vector3 normal = Vector3CrossProduct({ d.wallDir.x, 0, d.wallDir.y }, { 0, 1, 0 });

		Vector3 bottomLeft = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, -16));
		Vector3 bottomRight = Vector3Add({ d.pos.x, d.pos.y - 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, 16));
		Vector3 topLeft = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, -16));
		Vector3 topRight = Vector3Add({ d.pos.x, d.pos.y + 16, d.pos.z }, Vector3Scale({ d.wallDir.x, 0, d.wallDir.y }, 16));

		float tileX = 1.0f;
		float tileY = 1.0f;

		rlBegin(RL_QUADS);
		rlNormal3f(-normal.x, -normal.y, -normal.z);
		rlSetTexture(Shared::textures[d.textureName].id);

		rlColor4ub(255, 255, 255, 255);

		rlTexCoord2f(0.0f, 0.0f);  rlVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
		rlTexCoord2f(0.0f, tileY); rlVertex3f(topLeft.x, topLeft.y, topLeft.z);
		rlTexCoord2f(tileX, tileY); rlVertex3f(topRight.x, topRight.y, topRight.z);
		rlTexCoord2f(tileX, 0.0f);  rlVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

		rlEnd();
		rlSetTexture(0);
	}
}

void RealEditor::UpdateTrigger()
{

	if (!camMode) return;//Player Cam Mode.

	//---BoxTriggers----
	for (auto& [name, trigger] : Shared::boxTriggers) {

		//Update the bounding box of the trigger
		trigger.UpdateBox();

		if (trigger.IsRayHit(Shared::freeRay) && IsKeyPressed(KEY_C)) {
			Shared::selectedBoxTrigger = &trigger; 
		}
	}

	//Deselect
	if (IsKeyReleased(KEY_C))
	{
		Shared::selectedBoxTrigger = nullptr;
		freeCamToggle = true;
	}
	
	//Delete Trigger
	for (auto& [name, trigger] : Shared::boxTriggers) {
		if(trigger.IsRayHit(Shared::freeRay) && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		{
			Shared::boxTriggers.erase(name);
			Shared::selectedBoxTrigger = nullptr;
			freeCamToggle = true;
			break;
		}
	}

	//Move the selected trigger
	if (Shared::selectedBoxTrigger != nullptr) {
		freeCamToggle = false;

		//Move the Trigger and its collider
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
		Shared::selectedBoxTrigger->pos = Vector3Add(Shared::selectedBoxTrigger->pos, { 0,move.y, 0 });

		//Updating XZ and check Wall collisons

		Shared::selectedBoxTrigger->pos = Vector3Add(Shared::selectedBoxTrigger->pos, { move.x, 0, move.z });
	}

}

void RealEditor::DrawTrigger()
{
	if (!camMode) return;//Player Cam Mode.

	for(auto& [name, trigger] : Shared::boxTriggers) {

		DrawBoundingBox(trigger.box, BLUE);
	}
}

bool RealEditor::WallCloserThanSector()
{
	float wallDist = FLT_MAX;
	float sectorDist = FLT_MAX;

	for (auto& wall : Shared::walls)
	{
		Vector3 topLeft = { wall->startPos.x, (wall->wallHeight * wall->floor) + wall->wallHeight, wall->startPos.y };
		Vector3 bottomLeft = { wall->startPos.x,  wall->wallHeight * wall->floor,                    wall->startPos.y };
		Vector3 topRight = { wall->endPos.x,   (wall->wallHeight * wall->floor) + wall->wallHeight,  wall->endPos.y };
		Vector3 bottomRight = { wall->endPos.x,    wall->wallHeight * wall->floor,                   wall->endPos.y };

		RayCollision col = GetRayCollisionQuad(Shared::freeRay, topLeft, bottomLeft, bottomRight, topRight);
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

			RayCollision col = GetRayCollisionTriangle(Shared::freeRay, p0, p1, p2);
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

	for (auto& [id, worldObject] : Shared::worldObjects) {
		for(auto& box: worldObject.collisionBoxes) {
			if (CheckCollisionBoxes(box, Shared::player.collider)) {
				return true;
			}
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

	for (auto& [id, worldObject] : Shared::worldObjects) {
		for (auto& box : worldObject.collisionBoxes) {
			if (CheckCollisionBoxes(box, Shared::player.collider)) {

				float eBottom = box.min.y;
				float eTop = box.max.y;

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
}

Wall* RealEditor::GetRayWall(Ray ray){

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

	return closestWall;
}
