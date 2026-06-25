#include "TileEditor.h"
#include "raymath.h"

TileEditor::TileEditor()
{
	
	rows = worldRows;
	cols = worldCols;

	InitCamera();
}

void TileEditor::InitCamera()
{
	cam.offset = { 0 };
	cam.target = { 0 };
	cam.zoom = 1;
}

void TileEditor::Update()
{
	mouse = GetScreenToWorld2D(GetMousePosition(), cam);


	//Update Camera
	UpdateCam();

	//Update Walls
	UpdateWall();
	
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
	//Getting the Tile that is colliding with the mouse in grid space.
	int tileX = (int)mouse.x / tileSize;
	int tileY = (int)mouse.y / tileSize;

	//Placing Walls
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		
		pendingWall.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };

	}
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {

		pendingWall.endPos = { (float)(tileX * tileSize + tileSize / 2), (float)(tileY * tileSize + tileSize / 2) };
		walls.push_back(pendingWall);
	}

	//Removing Walls
	for (int i = 0; i < walls.size(); i++) {
		if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && CheckCollisionPointLine(mouse, walls[i].startPos, walls[i].endPos, 4))
			walls.erase(walls.begin() + i);
	}

	//Update Place Holder Wall
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		placeHolder.startPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
		placeHolder.endPos = { (float)(tileX * tileSize + tileSize / 2) , (float)(tileY * tileSize + tileSize / 2) };
	else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
		placeHolder = { 0,0 };
}

void TileEditor::DrawWall()
{
	for (Wall& wall: walls) {
		DrawLine(wall.startPos.x, wall.startPos.y, wall.endPos.x, wall.endPos.y, PURPLE);
	}
	DrawLine(placeHolder.startPos.x, placeHolder.startPos.y, placeHolder.endPos.x, placeHolder.endPos.y, PURPLE);
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

}
