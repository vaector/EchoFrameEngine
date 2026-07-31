#pragma once
#include "Entity.h"
#include <unordered_map>

class NPC : public Entity
{

public:
	std::string name = "";//The NPC name(Also the model name)
	ModelAnimation* modelAnim = nullptr;//A pointer to all the animations from pur model's .GLB
	Model model;//Model

	std::unordered_map<int, bool> isPlayingMap;//A map of the animations playing state. 
	std::unordered_map<int, bool> isFinishedMap;//A map of the animations playing state. 

	int animCount = 0;//How many animations?
	float animSpeed = 1.0f;//Animation Speed
	std::unordered_map<int, float> currentAccumulatorMap; //Accumulate time to proceed to the next frame
	std::unordered_map<int, int> currentFrameMap; //Points at an animation and stores its current frame
	int currentAnim = -1;

	void playAnim(int& currentAnim, bool& isLoop);
	void stopAnim();
	
};

