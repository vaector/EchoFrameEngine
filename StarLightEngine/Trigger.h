#pragma once
#include "raylib.h"
#include <functional>
#include "raymath.h"
#include <string>

class Trigger
{
public:
	Vector3 pos = { 0 };//Position of the trigger in 3D space
	std::string name = "";//Trigger Name to identify the one Trigger from another

};

