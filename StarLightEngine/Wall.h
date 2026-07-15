#pragma once
#include "raylib.h"
#include <unordered_map>

struct Wall{ //Wall|Door(A wall can be a door)
	Vector2 startPos = { 0,0 };
	Vector2 endPos = { 0,0 };
	float wallHeight = 0;
	int floor = 0;
	Texture2D* texture = nullptr;
	bool inSector = false;
	bool isDoor = false;
	bool isOpen = false;
};

struct DoorSwitch {
	Vector3 pos = { 0 };
	int floor = 0;
	Texture2D* texture = nullptr;
	Wall* linkedWall = nullptr;

};

struct Sector{
	std::vector<Vector2> floorPoints;
	std::vector<Vector2> ceilingPoints;
	int floor = 0;
	Texture2D* floorTexture = nullptr; 
	Texture2D* ceilingTexture = nullptr;
	bool hasFloor = true;
	bool hasCeiling = true;
};