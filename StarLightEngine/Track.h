#pragma once
#include "NPC.h"
#include <vector>
#include <string>

struct KeyFrame {
	int frame;
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
};

class Track
{
public:
	std::vector<KeyFrame> keyframes;
	float time = 0.0f;
	size_t currentFrame = 0;
};

