#pragma once
#include "Track.h"

class Cutscene
{

public:
	std::unordered_map<int, std::vector<Track>> entityCutscene;//Key-> Entity ID
	std::unordered_map<std::string, std::vector<Track>> npcCutscene;//Key-> NPC Name

	void Play();

};

