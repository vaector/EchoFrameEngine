#pragma once
#include "raylib.h"
struct Wall{
	Vector2 startPos;
	Vector2 endPos;
	int wallHeight;
	int floor;
	Texture2D* texture;
	bool inSector = false;
};

struct Sector{
	std::vector<Vector2> points;
	int floor;
	Texture2D* texture;
};