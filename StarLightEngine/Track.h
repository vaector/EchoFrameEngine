#pragma once
#include "NPC.h"
#include <vector>
#include <string>

struct KeyFrame {
	int frame;
	Vector3 position;
	Vector3 rotationAxis;
	float rotationAngle;
	Vector3 scale;
};

struct Track
{
	std::vector<KeyFrame> keyframes;
	KeyFrame originalKeyFrame;
	float time = 0.0f;
	int currentFrame = 0;
	bool finished = false;
	float animDuration = 1.0f;//1 second default
	std::string name;
	int worldObjectID = -1; // ID of the entity this track belongs to	
	std::string npcName = ""; // Name of the NPC this track belongs to, if applicable
};

