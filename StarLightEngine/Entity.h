#pragma once
#include <string>
#include "raylib.h"

class Entity
{
public:
	Vector3 position = {0};
	std::string modelName = "";
	Vector3 scale = {80,80,80};
	BoundingBox collider = {0};

};

