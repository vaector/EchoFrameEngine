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

//Cable Gun
enum CableState
{
	FirstEndpointAttached,
	Connected
};

struct Cable
{
	std::string startSocket = "";
	std::string endSocket = "";
	CableState state = FirstEndpointAttached;
};

class Hand
{
private:
	std::string textureName = "";
	Fingers currentFinger = IndexFinger;

	int pendingCableIndex = -1;
	std::string GetHitSocketName();

public:

	void CreateHand(std::string name);

	void UpdateHand();
	void UpdateNailShooting();
	void UpdateCableShooting();

	void UpdateFinger();

	void DrawHand();
	void DrawCable();
};

