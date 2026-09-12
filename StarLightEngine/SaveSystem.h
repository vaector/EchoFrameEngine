#pragma once
#include <string>
#include "json.hpp"

using json = nlohmann::json;

class SaveSystem
{

public:
	static void SaveGame(const std::string& filename);
	static void LoadGame(const std::string& filename);

private:
	static void SaveWalls(json& j);
	static void LoadWalls(const json& j);

	static void SaveSectors(json& j);
	static void LoadSectors(const json& j);

	static void SaveRamps(json& j);
	static void LoadRamps(const json& j);

	static void SaveDoorSwitches(json& j);
	static void LoadDoorSwitches(json& j);

	static void SaveWorldObjects(json& j);
	static void LoadWorldObjects(json& j);

	static void SaveNPCs(json& j);
	static void LoadNPCs(json& j);

	static void SaveCutscenes(json& j);
	static void LoadCutscenes(json& j);

	static void SavePlayer(json& j);
	static void LoadPlayer(json& j);

	static void SaveLights(json& j);
	static void LoadLights(const json& j);

	static void SaveDecals(json& j);
	static void LoadDecals(const json& j);

};

