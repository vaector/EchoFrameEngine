#pragma once
#include "raylib.h"
#include <string>
#include "Wall.h"

struct DoorSwitch {
	Vector3 pos = { 0 };
	int floor = 0;
	std::string  textureName = "";
	Wall* linkedWall = nullptr;
	int wallId = -1;
	int layer = 0;
};