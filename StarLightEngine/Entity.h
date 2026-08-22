#pragma once
#include <string>
#include "raylib.h"

class Entity
{
public:
	Vector3 position = {0};
	std::string modelName = "";
	Vector3 scale = {50,50,50};
	Vector3 rotationAxis = {0};
	float rotationAngle = 0.0f;
	BoundingBox collider = {0};
	BoundingBox originalCollider = { 0 };
};

