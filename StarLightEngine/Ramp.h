#pragma once
#include "raylib.h"

struct Ramp {
	Vector2 startPos;
	Vector2 endPos;
	Texture2D* texture = nullptr;
	int floor;
	float rampHeight;
	int halfWidth = 50;
	int layer = 0;
};