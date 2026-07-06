#include "TileEditor.h"

#include "imgui/imgui.h"
#include "raylib.h"
#include "raymath.h"
#include "Shared.h"
#include <vector>


TileEditor::TileEditor()
{
	
	rows = worldRows;
	cols = worldCols;

	InitCamera();
}

void TileEditor::InitCamera()
{
	cam.rotation = 0.0f;
	cam.target = { 0 };
	cam.offset = { 0, 0};
	cam.zoom = 1;
}

void TileEditor::Update()
{
	mouse = GetScreenToWorld2D(GetMousePosition(), cam);
	Shared::onImguiWindow = (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered());

	//Update Camera
	UpdateCam();

	//Select wall
	UpdateSelectedWall();

	//Update Walls and Vertives(UpdateWallVertices)
	if(!UpdateWallVertices())
		UpdateWall();

	//Update Sector
	UpdateSector();

	//Update Ramp
	UpdateRamp();

	//Door/Switch Update
	UpdateDoor();
	UpdateDoorSwitch();
	
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
		pendingWall.inSector = !Shared::sectorMode;
		//if sectorMode == true then inSector is false(false because the wall is not in a sector and should be added to one).
		Shared::walls.push_back(new Wall(pendingWall));
	}

	
	//Removing Walls
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		for (int i = 0; i < Shared::walls.size(); i++) {
			if (CheckCollisionPointLine(mouse, Shared::walls[i]->startPos, Shared::walls[i]->endPos, 4)) {

				//Remove connection that the wall has with a door switch if it is linked to one.
				for (int j = 0; j < Shared::doorSwitches.size(); j++) {
					if(Shared::doorSwitches[j].linkedWall == Shared::walls[i])
						Shared::doorSwitches.erase(Shared::doorSwitches.begin() + j);
				}

				//Deselecting the wall if it is selected before deleting it.
				if(selectedWall == Shared::walls[i])
					selectedWall = nullptr;
				
				delete Shared::walls[i];
				Shared::walls[i] = nullptr;
				Shared::walls.erase(Shared::walls.begin() + i);

				break;
			}
		}
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

	//Removing Sector
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		for (int j = 0; j < Shared::sectors.size(); j++) {
			if (Shared::sectors[j].floor == Shared::floor) {
				for (int i = 1; i < Shared::sectors[j].floorPoints.size() - 1; i++) {
					Vector2 p0 = Shared::sectors[j].floorPoints[0];
					Vector2 p1 = Shared::sectors[j].floorPoints[i];
					Vector2 p2 = Shared::sectors[j].floorPoints[i + 1];
					if (CheckCollisionPointTriangle(mouse, p0, p1, p2)) {
						Shared::sectors.erase(Shared::sectors.begin() + j);
						return;
					}
				}
			}
		}
	}

	//Add Sector
	if (IsKeyPressed(KEY_C)) {
		std::vector<Vector2> currentSector;
		bool first = false;
		for (auto& wall : Shared::walls) {
			if (!wall->inSector) {
				if (first == false) {
					currentSector.push_back(wall->startPos);
					first = true;
				}
				currentSector.push_back(wall->endPos);
				wall->inSector = true;
			}
		}

		if (currentSector.size() >= 3)
			Shared::sectors.push_back({ currentSector, currentSector, Shared::floor, Shared::selectedTexture, Shared::selectedTexture });
	
	}

	//Resize Sector
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;
	Vector2 tile = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };
	const float grabRadius = tileSize * 0.25f;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

		draggingSectorVertices.clear();
		for (auto& sector : Shared::sectors) {
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


	//Placing Ramp
	if (IsKeyPressed(KEY_R)) {

		pendingRamp.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };

	}
	else if (IsKeyReleased(KEY_R)) {
		
		Vector2 tile = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };;
		
		if (Vector2Distance(pendingRamp.startPos, tile) <= tileSize*3)
			return;

		pendingRamp.endPos = tile;
		pendingRamp.texture = Shared::selectedTexture;
		pendingRamp.floor = Shared::floor;
		pendingRamp.rampHeight = Shared::wallHeight;
		
		Shared::ramps.push_back(pendingRamp);
	}

	//Removing Ramp
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		for (int i = 0; i < Shared::ramps.size(); i++) {
			if (CheckCollisionPointLine(mouse, Shared::ramps[i].startPos, Shared::ramps[i].endPos, 4))
				Shared::ramps.erase(Shared::ramps.begin() + i);
		}
	}

	//Update Place Holder Ramp
	if (IsKeyPressed(KEY_R))
		placeHolderRamp.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	if (IsKeyDown(KEY_R))
		placeHolderRamp.endPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	else if (IsKeyReleased(KEY_R))
		placeHolderRamp = { 0,0 };

}

void TileEditor::UpdateDoor()
{
	if(!Shared::doorMode) return;

	for (auto& wall: Shared::walls) {
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
						newSwitch.linkedWall = selectedWall;
						//Pos
						Vector3 wallCenter = { (selectedWall->startPos.x + selectedWall->endPos.x) / 2, selectedWall->wallHeight / 2 ,(selectedWall->startPos.y + selectedWall->endPos.y) / 2 };
						newSwitch.pos = wallCenter;

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

	//Delete Door Switch
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
		for (int i = 0; i < Shared::doorSwitches.size(); i++) {
			if(Shared::doorSwitches[i].floor != Shared::floor) continue;

			if (CheckCollisionPointRec(mouse, { Shared::doorSwitches[i].pos.x-2, Shared::doorSwitches[i].pos.z-2, 4,4 })) {
				Shared::doorSwitches.erase(Shared::doorSwitches.begin() + i);
				break;
			}
		}
	}

}

void TileEditor::UpdateSelectedWall()
{
	for (auto& wall : Shared::walls) {
		if(wall->floor == Shared::floor)
		if(CheckCollisionPointLine(mouse, wall->startPos, wall->endPos, 2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selectedWall = wall;
			break;
		}
	}

	//Deselect Wall
	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
		selectedWall = nullptr;
}

void TileEditor::DrawDoorSwitch()
{
	for(auto& doorSwitch: Shared::doorSwitches) {
		if (doorSwitch.floor != Shared::floor) continue;

		DrawRectangle(doorSwitch.pos.x - 2, doorSwitch.pos.z - 2, 4, 4, RED);
	}
}

void TileEditor::DrawWall()
{
	for (auto& wall: Shared::walls) {
		if (wall->floor != Shared::floor) continue;
		//Draw Wall/Door
		if(wall->isDoor)
			DrawLine(wall->startPos.x, wall->startPos.y, wall->endPos.x, wall->endPos.y, ORANGE);
		else
			DrawLine(wall->startPos.x, wall->startPos.y, wall->endPos.x, wall->endPos.y, PURPLE);
		//Draw Verices
		DrawCircle(wall->startPos.x, wall->startPos.y, 2, BLUE);
		DrawCircle(wall->endPos.x, wall->endPos.y, 2, BLUE);
	}
	DrawLine(placeHolderWall.startPos.x, placeHolderWall.startPos.y, placeHolderWall.endPos.x, placeHolderWall.endPos.y, PURPLE);
}

void TileEditor::DrawSector()
{
	for (auto& sector : Shared::sectors) {
		if (sector.floor != Shared::floor) continue;

		DrawTriangleFan(sector.ceilingPoints.data(), sector.ceilingPoints.size(), {255,0,0,50});
	}
}

void TileEditor::DrawRamp()
{
	for (Ramp& ramp : Shared::ramps) {
		if (ramp.floor != Shared::floor) continue;
		DrawLine(ramp.startPos.x, ramp.startPos.y, ramp.endPos.x, ramp.endPos.y, GREEN);
	}
	DrawLine(placeHolderRamp.startPos.x, placeHolderRamp.startPos.y, placeHolderRamp.endPos.x, placeHolderRamp.endPos.y, GREEN);
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

}
