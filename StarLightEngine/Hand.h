#pragma once
#include "Shared.h"

class Hand
{
private:
	std::string textureName;
public:

	void CreateHand(std::string name);
	void UpdateHand();
	void UpdateNailShooting();
	void DrawHand();
};

