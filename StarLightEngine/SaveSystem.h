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

	static void SaveEntities(json& j);
	static void SaveNPCs(json& j);

};

