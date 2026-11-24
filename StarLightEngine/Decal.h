#pragma once
#include "raylib.h"
#include <string>

class Decal
{
public:
	 Vector3 pos = { 0 };
    Vector3 axisH = { 1, 0, 0 }; // Horizontal direction
    Vector3 axisV = { 0, 1, 0 }; // Vertical direction

    float scale = 1.f;
    int floor = 0;
    std::string textureName = "";
};

