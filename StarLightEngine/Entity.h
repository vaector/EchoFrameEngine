#pragma once
#include <string>
#include "raylib.h"

class Entity
{
public:
	int id = 0;
	Vector3 position = {0};
	std::string modelName = "";
	Vector3 scale = {50,50,50};
	Vector3 rotation = {0};
	BoundingBox collider = {0};
	BoundingBox originalCollider = { 0 };
	static int currentId;
};

