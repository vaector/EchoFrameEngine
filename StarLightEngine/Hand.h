#pragma once
#include "Shared.h"

//Fingers
enum Fingers
{
	IndexFinger,//Nail Gun
	MiddleFinger,
	RingFinger,//Cable Gun
	PinkyFinger
};

class Hand
{
private:
	std::string textureName = "";
	Fingers currentFinger = IndexFinger;

	int pendingCableIndex = -1;

	//Helpers
	std::string GetHitSocketName();
	bool IsSocketUsed(const std::string& socketName);

public:

	void CreateHand(std::string name);

	void UpdateHand();
	void UpdateNailShooting();
	void UpdateCableShooting();

	void UpdateFinger();

	void DrawHand();
	void DrawCable();

};

