#pragma once
#include <string>
#include "raylib.h"

class Entity
{
public:
	Vector3 position = {0};
	std::string modelName = "";
	Vector3 scale = {50,50,50};
	BoundingBox collider = {0};

};

