#pragma once
#include "raylib.h"
#include <string>

struct Ramp {
	Vector2 startPos;
	Vector2 endPos;
	std::string textureName = "";
	int floor;
	float rampHeight;
	int halfWidth = 50;
	int layer = 0;
};