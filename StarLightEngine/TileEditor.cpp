#include "TileEditor.h"

#include "imgui/imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "Shared.h"
#include <vector>

//Enabling earcut.hpp to use raylib's Vector2
#include "external/earcut.hpp"
namespace mapbox {
	namespace util {

		template <>
		struct nth<0, Vector2>
		{
			static double get(const Vector2& p)
			{
				return p.x;
			}
		};

		template <>
		struct nth<1, Vector2>
		{
			static double get(const Vector2& p)
			{
				return p.y;
			}
		};

	}
}
//Set the static wallId
int Wall::currentWallId = 0;

TileEditor::TileEditor()
{
	
	rows = worldRows;
	cols = worldCols;

	InitCamera();
}

void TileEditor::InitCamera()
{
	cam.rotation = 0.0f;
	cam.target = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	cam.offset = { 0, 0};
	cam.zoom = 1;
}

void TileEditor::Update()
{
	mouse = GetScreenToWorld2D(GetMousePosition(), cam);
	Shared::onImguiWindow = (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered());

	if (Shared::onImguiWindow) return;

	//Update Camera
	UpdateCam();

	//Select wall
	UpdateSelectedWall();

	//Update Walls and Vertives(UpdateWallVertices)
	if(!UpdateWallVertices() && WallMode)
		UpdateWall();

	//Update Sector
	UpdateSector();

	//Update Ramp
	UpdateRamp();

	//Door/Switch Update
	UpdateDoor();
	UpdateDoorSwitch();
	UpdatePlayerPos();

	SplitWall();

	//Remove
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		//Remove Wall/Sector/Ramp/DoorSwitch
		if (!RemoveWall() && !RemoveDoorSwitch() && !RemoveRamp())
			RemoveSector();

	}

}

void TileEditor::UpdateCam()
{
	float dt = GetFrameTime();

	//Move
	if (IsKeyDown(KEY_W)) cam.target.y -= dt * panSpeed;
	else if (IsKeyDown(KEY_S)) cam.target.y += dt * panSpeed;
	if (IsKeyDown(KEY_A)) cam.target.x -= dt * panSpeed;
	else if (IsKeyDown(KEY_D)) cam.target.x += dt * panSpeed;

	//Zoom
	if (GetMouseWheelMove() != 0) {
		float mouseWheel = GetMouseWheelMove();
		

		cam.zoom += mouseWheel *0.2f;
		cam.zoom = Clamp(cam.zoom, 0.3f, 10.0f);
		
		cam.offset = GetMousePosition();
		cam.target = mouse;
	}

	
}

void TileEditor::UpdateWall()
{
	if (Shared::onImguiWindow || IsKeyDown(KEY_R))//"R" creating ramp so do not add wall.
		return;

	//Getting the Tile that is colliding with the mouse in grid space.
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;

	//Placing Walls
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		
		pendingWall.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	}
	else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		pendingWall.endPos = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };
	
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && placeHolderWall.startPos != placeHolderWall.endPos) {
		Vector2 currentTile = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };;
		pendingWall.endPos = currentTile;
		pendingWall.wallHeight = Shared::wallHeight;
		pendingWall.floor = Shared::floor;
		pendingWall.texture = Shared::selectedTexture;
		pendingWall.textureName = *Shared::selectedTextureName;
		pendingWall.inSector = !Shared::sectorMode;
		pendingWall.layer = Shared::layer;
		pendingWall.id = Wall::currentWallId;
		Wall::currentWallId++;
		//if sectorMode == true then inSector is false(false because the wall is not in a sector and should be added to one).
		Shared::walls.push_back(new Wall(pendingWall));
	}

	//Update Place Holder Wall
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		placeHolderWall.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		placeHolderWall.endPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		placeHolderWall = { 0,0 };
}

bool TileEditor::UpdateWallVertices()
{
	if (Shared::onImguiWindow)
		return false;

	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;
	Vector2 tile = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };

	const float grabRadius = tileSize * 0.25f; 

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		draggingVertices.clear();
		for (auto& wall : Shared::walls) {
			if (wall->floor != Shared::floor || wall->layer != Shared::layer) continue;

			if (CheckCollisionPointCircle(mouse, wall->startPos, grabRadius))
				draggingVertices.push_back(&wall->startPos);
			if (CheckCollisionPointCircle(mouse, wall->endPos, grabRadius))
				draggingVertices.push_back(&wall->endPos);
		}
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !draggingVertices.empty()) {
		for (Vector2* v : draggingVertices)
			*v = tile;
		placeHolderWall = { 0, 0 };
		return true;
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		draggingVertices.clear();

	return !draggingVertices.empty();
}

void TileEditor::UpdateSector()
{
	if (Shared::onImguiWindow || Shared::walls.size() <= 2)
		return;

	//Add Sector
	if (IsKeyPressed(KEY_C)) {
		std::vector<Vector2> currentSector;
		bool first = false;
		for (auto& wall : Shared::walls) {
			if (wall->floor != Shared::floor) continue;

			if (!wall->inSector) {
				if (first == false) {
					currentSector.push_back(wall->startPos);
					first = true;
				}
				currentSector.push_back(wall->endPos);
				wall->inSector = true;
			}
		}

		if (currentSector.size() >= 3) {

			// Convert to Earcut format
			std::vector<std::vector<Vector2>> polygon;
			polygon.push_back(currentSector);

			Sector sector;
			sector.floorPoints = currentSector;
			sector.ceilingPoints = currentSector;
			sector.indices = mapbox::earcut<int>(polygon);

			sector.floor = Shared::floor;
			sector.floorTexture = Shared::selectedTexture;
			sector.floorTextureName = *Shared::selectedTextureName;
			sector.ceilingTexture = Shared::selectedTexture;
			sector.ceilingTextureName = *Shared::selectedTextureName;
			sector.layer = Shared::layer;

			if (Shared::ceilingOnly) {
				sector.hasFloor = false;
				sector.hasCeiling = true;
			}
			else if (Shared::floorOnly) {
				sector.hasFloor = true;
				sector.hasCeiling = false;
			}
			else {
				sector.hasFloor = true;
				sector.hasCeiling = true;
			}

			Shared::sectors.push_back(sector);
		}

	}

	//Resize Sector
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;
	Vector2 tile = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };
	const float grabRadius = tileSize * 0.25f;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

		draggingSectorVertices.clear();
		for (auto& sector : Shared::sectors) {
			if (sector.floor != Shared::floor || sector.layer != Shared::layer) continue;
			for (int i = 0; i < sector.floorPoints.size(); i++) {
				if (CheckCollisionPointCircle(mouse, sector.floorPoints[i], grabRadius)) {
					draggingSectorVertices.push_back(&sector.floorPoints[i]);
					draggingSectorVertices.push_back(&sector.ceilingPoints[i]);
				}
			}
		}
	}

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !draggingSectorVertices.empty()) {
		for (Vector2* point : draggingSectorVertices)
			*point = tile;
	}

	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		draggingSectorVertices.clear();
	
}

void TileEditor::UpdateRamp()
{
	if (Shared::onImguiWindow)
		return;

	//Getting the Tile that is colliding with the mouse in grid space.
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;
	Vector2 tile = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };

	//Placing Ramp
	if (IsKeyPressed(KEY_R)) {

		float dist = FLT_MAX;
		Vector2* closestFloorPointStart = nullptr;
		Vector2* closestFloorPointEnd = nullptr;
		for (auto& sector: Shared::sectors) {
			if (sector.floor != Shared::floor) continue;

			for (int i = 0; i < sector.floorPoints.size(); i++) {
				Vector2& p0 = sector.floorPoints[i];
				Vector2& p1 = sector.floorPoints[(i + 1) % sector.floorPoints.size()];

				float d = Vector2Distance(mouse, ClosestPointOnLineSegment(mouse, p0, p1));
				if (d < dist) {
					dist = d;
					closestFloorPointStart = &p0;
					closestFloorPointEnd = &p1;
				}
			}
		}
		
		//If there is a close wall then add the startPos of the  ramp to the closest point on the wall, otherwise add it to the tile.
		if ((closestFloorPointStart != nullptr || closestFloorPointEnd != nullptr) && dist < 12) {
			pendingRamp.startPos = ClosestPointOnLineSegment(mouse, *closestFloorPointStart, *closestFloorPointEnd);
			placeHolderRamp.startPos = ClosestPointOnLineSegment(mouse, *closestFloorPointStart, *closestFloorPointEnd);
		}
		else {
			
		}

	}
	else if (IsKeyReleased(KEY_R)) {
		
		if (Vector2Distance(pendingRamp.startPos, tile) <= tileSize*3)
			return;

		pendingRamp.endPos = tile;
		pendingRamp.texture = Shared::selectedTexture;
		pendingRamp.floor = Shared::floor;
		pendingRamp.rampHeight = Shared::wallHeight;
		pendingRamp.layer = Shared::layer;
		pendingRamp.textureName = *Shared::selectedTextureName;
		
		Shared::ramps.push_back(pendingRamp);

	}

	//Update Place Holder Ramp
	if (IsKeyDown(KEY_R))
		placeHolderRamp.endPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	else if (IsKeyReleased(KEY_R))
		placeHolderRamp = { 0,0 };


	//Resize Ramp
	static Ramp* selectedResizeRamp = nullptr;
	for (auto& ramp: Shared::ramps) {
		if (ramp.floor != Shared::floor || ramp.layer != Shared::layer) continue;

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector2 perp = Vector2Scale({ -dir.y, dir.x }, ramp.halfWidth);

		Vector2 startLeft = Vector2Subtract(ramp.startPos, perp);
		Vector2 startRight = Vector2Add(ramp.startPos, perp);
		Vector2 endLeft = Vector2Subtract(ramp.endPos, perp);
		Vector2 endRight = Vector2Add(ramp.endPos, perp);

		if ((CheckCollisionPointLine(mouse, startLeft, endLeft, 4) || CheckCollisionPointLine(mouse, startRight, endRight, 4)) && IsKeyPressed(KEY_C)) {
			selectedResizeRamp = &ramp;
			WallMode = false;
		}
		if(selectedResizeRamp && IsKeyDown(KEY_C)) {
			
			if(IsKeyPressed(KEY_RIGHT))
				selectedResizeRamp->halfWidth += tileSize;
			else if (IsKeyPressed(KEY_LEFT))
				selectedResizeRamp->halfWidth -= tileSize;
			
		}
		else if (IsKeyReleased(KEY_C)) {
			selectedResizeRamp = nullptr;
			WallMode = true;
		}
	}

}

void TileEditor::UpdateDoor()
{
	if(!Shared::doorMode) return;

	for (int i = Shared::walls.size() - 1; i >= 0; i--) {
		auto& wall = Shared::walls[i];
		if (wall->floor != Shared::floor || wall->layer != Shared::layer) continue;

		if (CheckCollisionPointLine(mouse, wall->startPos, wall->endPos, 4) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			wall->isDoor = !wall->isDoor;

			//if wall is no longer a door , remove any door switches linked to it.
			if (!wall->isDoor) {
				for(int i = 0; i < Shared::doorSwitches.size(); i++) {
					if (Shared::doorSwitches[i].linkedWall == wall) {
						Shared::doorSwitches.erase(Shared::doorSwitches.begin() + i);
						break;
					}
				}
			}
			//Add Door Switch
			else {
				
				if (selectedWall != nullptr) {

					if (CheckCollisionPointLine(mouse, selectedWall->startPos, selectedWall->endPos, 2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
						DoorSwitch newSwitch;
						newSwitch.floor = Shared::floor;
						newSwitch.texture = Shared::selectedTexture;
						newSwitch.textureName = *Shared::selectedTextureName;
						newSwitch.linkedWall = selectedWall;
						newSwitch.wallId = selectedWall->id;
						//Pos
						Vector3 wallCenter = { (selectedWall->startPos.x + selectedWall->endPos.x) / 2, selectedWall->wallHeight / 2 ,(selectedWall->startPos.y + selectedWall->endPos.y) / 2 };
						newSwitch.pos = wallCenter;
						newSwitch.layer = newSwitch.linkedWall->layer;
						Shared::doorSwitches.push_back(newSwitch);
					}
				}
				break;
			}
		}
		
	}
}

void TileEditor::UpdateDoorSwitch()
{

	//Update Door Switch Position
	static DoorSwitch* selectedSwitch = nullptr;
	for (auto& d : Shared::doorSwitches) {
		if (d.floor != Shared::floor || d.layer != Shared::layer) continue;

		if (CheckCollisionPointRec(mouse, { d.pos.x-2, d.pos.z-2, 4,4 }) && IsKeyPressed(KEY_C)) {
			selectedSwitch = &d;
		}
	}

	if (IsKeyDown(KEY_C) && selectedSwitch != nullptr) {
		selectedSwitch->pos.x = mouse.x;
		selectedSwitch->pos.z = mouse.y;
	}
	else if (IsKeyReleased(KEY_C))
		selectedSwitch = nullptr;

}

void TileEditor::UpdateSelectedWall()
{
	for (auto& wall : Shared::walls) {
		if (wall->floor != Shared::floor || wall->layer != Shared::layer) continue;

		if(CheckCollisionPointLine(mouse, wall->startPos, wall->endPos, 2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selectedWall = wall;
			break;
		}
	}

	//Deselect Wall
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		selectedWall = nullptr;
}

void TileEditor::UpdatePlayerPos()
{
	static bool selected = false;
	if (CheckCollisionPointCircle(mouse, {Shared::playerPos.x, Shared::playerPos.z}, 6) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		selected = true;
		WallMode = false;
	}
	if (selected) {
		Shared::playerPos = { mouse.x, (float)Shared::floor * Shared::wallHeight, mouse.y };
	}
	if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		selected = false;
		WallMode = true;
	}
}

void TileEditor::SplitWall()
{
	if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || Shared::doorMode) return;

	//Getting the Tile that is colliding with the mouse in grid space.
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;
	Vector2 tile = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };


	for(int i = Shared::walls.size() - 1; i >= 0; i--) {
		auto& wall = Shared::walls[i];
		if (wall->floor != Shared::floor || wall->layer != Shared::layer) continue;

		Vector2 dir = Vector2Normalize(Vector2Subtract(wall->endPos, wall->startPos));

		if (CheckCollisionPointLine(mouse, Vector2Add(wall->startPos, Vector2Scale(dir, 4)), Vector2Subtract(wall->endPos, Vector2Scale(dir, 4)), 4)) {
			Vector2 oldEndPos = wall->endPos;
			wall->endPos = tile;

			Wall* newWall = new Wall();
			newWall->startPos = wall->endPos;
			newWall->endPos = oldEndPos;
			newWall->floor = wall->floor;
			newWall->wallHeight = wall->wallHeight;
			newWall->texture = wall->texture;
			newWall->textureName = wall->textureName;
			newWall->id = Wall::currentWallId;
			Wall::currentWallId++;

			wall->inSector = false;
			newWall->inSector = false;

			newWall->layer = wall->layer;
			
			Shared::walls.insert(Shared::walls.begin() + i + 1, newWall);
		}
	}


}

bool TileEditor::RemoveWall()
{
	//Removing Walls
	for (int i = 0; i < Shared::walls.size(); i++) {
		if (Shared::walls[i]->floor != Shared::floor || Shared::walls[i]->layer != Shared::layer) continue;

		if (CheckCollisionPointLine(mouse, Shared::walls[i]->startPos, Shared::walls[i]->endPos, 2)) {

			//Remove connection that the wall has with a door switch if it is linked to one.
			for (int j = 0; j < Shared::doorSwitches.size(); j++) {
				if (Shared::doorSwitches[j].linkedWall == Shared::walls[i])
					Shared::doorSwitches.erase(Shared::doorSwitches.begin() + j);
			}

			//Deselecting the wall if it is selected before deleting it.
			if (selectedWall == Shared::walls[i])
				selectedWall = nullptr;

			delete Shared::walls[i];
			Shared::walls[i] = nullptr;
			Shared::walls.erase(Shared::walls.begin() + i);

			return true;
		}
	}
	return false;
}

bool TileEditor::RemoveSector()
{
	//Removing Sector
	for (int j = 0; j < Shared::sectors.size(); j++) {
		if (Shared::sectors[j].floor != Shared::floor || Shared::sectors[j].layer != Shared::layer) continue;

		for (int i = 1; i < Shared::sectors[j].floorPoints.size() - 1; i++) {
			Vector2 p0 = Shared::sectors[j].floorPoints[0];
			Vector2 p1 = Shared::sectors[j].floorPoints[i];
			Vector2 p2 = Shared::sectors[j].floorPoints[i + 1];
			if (CheckCollisionPointTriangle(mouse, p0, p1, p2)) {

				//When deleting a sector make the wall.inSector = false so that we could use it again unless the wall itself is also deleted.
				for (int k = 0; k < Shared::walls.size(); k++) {
					auto& wall = Shared::walls[k];

					bool flagStartPos = false;
					bool flagEndPos = false;
					for (int l = 0; l < Shared::sectors[j].floorPoints.size(); l++) {
						auto& floorPoint = Shared::sectors[j].floorPoints[l];

						if (wall->startPos == floorPoint)
							flagStartPos = true;
					}
					for (int l = 0; l < Shared::sectors[j].floorPoints.size(); l++) {
						auto& floorPoint = Shared::sectors[j].floorPoints[l];

						if (wall->endPos == floorPoint)
							flagEndPos = true;
					}
					if (flagEndPos && flagStartPos)
						wall->inSector = false;
				}
				Shared::sectors.erase(Shared::sectors.begin() + j);
				return true;
			}
		}

	}
	return false;
}

bool TileEditor::RemoveRamp()
{
	//Removing Ramp
	for (int i = 0; i < Shared::ramps.size(); i++) {
		Ramp& ramp = Shared::ramps[i];

		if (ramp.floor != Shared::floor || ramp.layer != Shared::layer) continue;

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector2 perp = Vector2Scale({ -dir.y, dir.x }, (float)ramp.halfWidth);

		Vector2 startLeft = Vector2Subtract(ramp.startPos, perp);
		Vector2 startRight = Vector2Add(ramp.startPos, perp);
		Vector2 endLeft = Vector2Subtract(ramp.endPos, perp);
		Vector2 endRight = Vector2Add(ramp.endPos, perp);

		if (CheckCollisionPointLine(mouse, startLeft, endLeft, 4) ||
			CheckCollisionPointLine(mouse, startRight, endRight, 4) ||
			CheckCollisionPointLine(mouse, startLeft, startRight, 4) ||
			CheckCollisionPointLine(mouse, endLeft, endRight, 4)) {

			Shared::ramps.erase(Shared::ramps.begin() + i);
			return true;
		}
	}
	return false;
}

bool TileEditor::RemoveDoorSwitch()
{
	for (int i = 0; i < Shared::doorSwitches.size(); i++) {
		if (Shared::doorSwitches[i].floor != Shared::floor || Shared::doorSwitches[i].layer != Shared::layer) continue;

		if (CheckCollisionPointRec(mouse, { Shared::doorSwitches[i].pos.x - 2, Shared::doorSwitches[i].pos.z - 2, 4,4 })) {
			Shared::doorSwitches.erase(Shared::doorSwitches.begin() + i);
			return true;
		}
	}

	return false;
}

void TileEditor::DrawPlayerPos()
{
	DrawCircleV({Shared::playerPos.x, Shared::playerPos.z}, 6, BLUE);
}

void TileEditor::DrawDoorSwitch()
{
	for (int i = 0; i < Shared::doorSwitches.size(); i++) {
		DoorSwitch& doorSwitch = Shared::doorSwitches[i];
		if (doorSwitch.floor != Shared::floor) continue;

		DrawRectangle(doorSwitch.pos.x - 2, doorSwitch.pos.z - 2, 4, 4, RED);
		DrawText(std::to_string(i).c_str(), doorSwitch.pos.x + 4, doorSwitch.pos.z - 4, 2, WHITE);
	}
}

void TileEditor::DrawWall()
{
	for (auto& wall: Shared::walls) {
		
		if (wall->floor == Shared::floor) {
			//Draw Wall/Door
			if (wall->isDoor) {

				DrawLineEx(wall->startPos, wall->endPos,2 ,{ 255,161,0,255 });
				int index = -1;
				for (int i = 0; i < Shared::doorSwitches.size(); i++)
				{
					if(Shared::doorSwitches[i].linkedWall == wall)
						index = i;
				}
				if (index != -1) {
					std::string indexStr = std::to_string(index);
					DrawText(indexStr.c_str(), (wall->startPos.x + wall->endPos.x) / 2, (wall->startPos.y + wall->endPos.y) / 2, 2, { 255,255,255,255 });
					
				}
			}
			else
				DrawLineEx(wall->startPos, wall->endPos, 2, {200,122,255,255});
			
			//Draw Verices
			if (wall->layer == Shared::layer) {
				DrawCircle(wall->startPos.x, wall->startPos.y, 2, { 0,0,255,255 });
				DrawCircle(wall->endPos.x, wall->endPos.y, 2, { 0,0,255,255 });
			}
		}
		else if (wall->floor == Shared::floor - 1) {
			//Draw Wall/Door
			if (wall->isDoor)
				DrawLineEx(wall->startPos, wall->endPos, 2, {255,161,0,100});
			else
				DrawLineEx(wall->startPos, wall->endPos, 2, {200,122,255,100});
			
			//Draw Verices
			//DrawCircle(wall->startPos.x, wall->startPos.y, 2, {0,0,255,100});
			//DrawCircle(wall->endPos.x, wall->endPos.y, 2, { 0,0,255,100 });
		}
		
	}
	DrawLineEx(placeHolderWall.startPos, placeHolderWall.endPos, 2, PURPLE);
}

void TileEditor::DrawSector()
{
	for (auto& sector : Shared::sectors)
	{
		Color color;

		if (sector.floor == Shared::floor)
			color = { 255, 0, 0, 50 };
		else if (sector.floor == Shared::floor - 1)
			color = { 255, 0, 0, 10 };
		else
			continue;

		for (int i = 0; i < sector.indices.size(); i += 3)
		{
			DrawTriangle(
				sector.ceilingPoints[sector.indices[i]],
				sector.ceilingPoints[sector.indices[i + 2]],
				sector.ceilingPoints[sector.indices[i + 1]],
				color
			);
		}
	}
}

void TileEditor::DrawRamp()
{
	for (Ramp& ramp : Shared::ramps) {

		Vector2 dir = Vector2Normalize(Vector2Subtract(ramp.endPos, ramp.startPos));
		Vector2 perp = Vector2Scale({ -dir.y, dir.x }, ramp.halfWidth);

		Vector2 startLeft = Vector2Subtract(ramp.startPos, perp);
		Vector2 startRight = Vector2Add(ramp.startPos, perp);
		Vector2 endLeft = Vector2Subtract(ramp.endPos, perp);
		Vector2 endRight = Vector2Add(ramp.endPos, perp);

		if (ramp.floor == Shared::floor) {
			DrawLineEx(startLeft, startRight, 2, { 0,255,0,255 });  //low edge
			DrawLineEx(endLeft, endRight, 2, { 0,255,0,255 });      //high edge
			DrawLineEx(startLeft, endLeft, 2, { 150,255,150,255 });     //left side
			DrawLineEx(startRight, endRight, 2, { 150,255,150,255 });   //right side
		}
		else if (ramp.floor == Shared::floor - 1) {
			DrawLineEx(startLeft, startRight, 2, {0,255,0,100});  //low edge
			DrawLineEx(endLeft, endRight, 2, {0,255,0,100});      //high edge
			DrawLineEx(startLeft, endLeft, 2, {0,255,0,100});     //left side
			DrawLineEx(startRight, endRight, 2, {0,255,0,100});   //right side
		}
	}
	DrawLineEx(placeHolderRamp.startPos, placeHolderRamp.endPos, 2, GREEN);
}

void TileEditor::Draw()
{

	// Get the world position of the screen corners
	Vector2 topLeft = GetScreenToWorld2D({ 0, 0 }, cam);
	Vector2 bottomRight = GetScreenToWorld2D({ (float)GetScreenWidth(), (float)GetScreenHeight() }, cam);

	// Convert to tile indices
	int startX = (int)(topLeft.x / tileSize) - 1;
	int startY = (int)(topLeft.y / tileSize) - 1;
	int endX = (int)(bottomRight.x / tileSize) + 1;
	int endY = (int)(bottomRight.y / tileSize) + 1;

	// Clamp to world bounds
	startX = Clamp(startX, 0, cols);
	startY = Clamp(startY, 0, rows);
	endX = Clamp(endX, 0, cols);
	endY = Clamp(endY, 0, rows);

	//Drawing the Grid
	for (int y = startY; y < endY; y++)
		for (int x = startX; x < endX; x++) {

			Rectangle tile = { (float)(x * tileSize ), (float)(y * tileSize ), (float)tileSize, (float)tileSize };
			DrawRectangleLinesEx(tile, 1, {255,255,255, 50});

		}

	//Draw Walls
	DrawWall();

	DrawSector();

	DrawRamp();

	DrawDoorSwitch();

	DrawPlayerPos();

}

Vector2 TileEditor::ClosestPointOnLineSegment(Vector2 p, Vector2 a, Vector2 b)
{
	Vector2 ab = Vector2Subtract(b, a);

	float abLengthSq = Vector2DotProduct(ab, ab);

	// Line segment is a single point.
	if (abLengthSq <= 0.0001f)
		return a;

	float t = Vector2DotProduct(Vector2Subtract(p, a), ab) / abLengthSq;
	t = Clamp(t, 0.0f, 1.0f);

	return Vector2Add(a, Vector2Scale(ab, t));
}

bool TileEditor::PointInSector(Vector2 point, const std::vector<Vector2>& poly)
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

bool TileEditor::pointInWall(Vector2 point, Vector2 startPos, Vector2 endPos)
{
	if (CheckCollisionPointLine(point, startPos, endPos, 4))
		return true;

	return false;
}
