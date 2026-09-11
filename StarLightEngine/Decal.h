#pragma once
#include "raylib.h"
#include <string>

class Decal
{
public:
	Vector3 pos = {0};
	float scale = 1.f;
	int floor = 0;
	std::string  textureName = "";
	int wallID = -1;
};

