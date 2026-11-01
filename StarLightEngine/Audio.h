#pragma once
#include "raylib.h"
#include "Shared.h"

class Audio
{
private:


public:
	static void Play(Sound& sound, bool loop = false);
	static void Stop(Sound& sound);
	static void Play(Music& music);
};

//Play -> Plays a sound. If loop is true, it will play the sound in a loop based on the condition. If loop is false, it will play the sound once. 