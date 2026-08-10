#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>

struct Wall{ //Wall|Door(A wall can be a door)
	int id = 0;
	Vector2 startPos = { 0,0 };
	Vector2 endPos = { 0,0 };
	float wallHeight = 0;
	int floor = 0;
	Texture2D* texture = nullptr;
	std::string  textureName = "";
	bool inSector = false;
	bool isDoor = false;
	bool isOpen = false;
	int layer = 0;
	static int currentWallId;
};


struct DoorSwitch {
	Vector3 pos = { 0 };
	int floor = 0;
	Texture2D* texture = nullptr;
	std::string  textureName = "";
	Wall* linkedWall = nullptr;
	int wallId = -1;
	int layer = 0;
};

struct Sector{
	std::vector<Vector2> floorPoints;
	std::vector<Vector2> ceilingPoints;
	std::vector<int> indices = {};
	int floor = 0;
	Texture2D* floorTexture = nullptr; 
	Texture2D* ceilingTexture = nullptr;
	std::string  floorTextureName = "";
	std::string  ceilingTextureName = "";
	bool hasFloor = true;
	bool hasCeiling = true;
	int layer = 0;
};