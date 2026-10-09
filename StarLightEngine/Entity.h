#pragma once
#include <string>
#include "raylib.h"
#include <vector>

enum CollisionType {
	NONE,//No collision
	BOX,//A simple Box
	MESH//Many meshes where each has its own box
};

class Entity
{
public:
	Vector3 position = {0};
	std::string modelName = "";
	Vector3 scale = {50,50,50};
	Vector3 rotationAxis = {0};
	float rotationAngle = 0.0f;

	CollisionType collisionType = NONE;
	std::vector<BoundingBox> collisionBoxes;//The Collision Box for all the mesh in model(if only 1 then it is a BOX).
	std::vector<BoundingBox> originalBoxes;//The Original state of the Box Colliders


};

