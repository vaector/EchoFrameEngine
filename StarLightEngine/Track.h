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

struct Track
{
	std::vector<KeyFrame> keyframes;
	KeyFrame originalKeyFrame;
	float time = 0.0f;
	int currentFrame = 0;
	std::string name;
};

