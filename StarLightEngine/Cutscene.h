#pragma once
#include "Track.h"
#include <string>

class Cutscene
{

public:
	std::unordered_map<int, std::vector<Track>> worldObjectCutscene;//Key-> World Object ID
	std::unordered_map<std::string, std::vector<Track>> npcCutscene;//Key-> NPC Name

	std::string playingTrack = "";


	void Play(std::string trackName);
	void Reset(std::string trackName);
	void SaveOriginalKeyFrame();
	void Stop();

};

