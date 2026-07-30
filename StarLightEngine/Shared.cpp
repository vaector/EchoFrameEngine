#include "Shared.h"


namespace Shared {
	bool editorMode = true;
	std::vector<Wall*> walls;
	int wallHeight = 200;
	std::unordered_map<std::string, Texture2D> textures;
	std::unordered_map<std::string, Model> models;
	Texture2D* selectedTexture = &textures["tile1"];
	std::string* selectedModelName = nullptr;
	std::vector<Entity> entities;
	std::vector<Sector> sectors;
	bool sectorMode = true;
	bool floorOnly = false;
	bool ceilingOnly = false;
	int floor = 0;
	bool onImguiWindow = false;
	std::vector<Ramp> ramps;
	float selectedRampHeight = 0.0f;
	bool doorMode = false;
	std::vector<DoorSwitch> doorSwitches;
	Vector3 playerPos = { 1024,0,1024};
	bool setPlayerPos = false;
	int layer = 0;

	bool npcMode = false;
	std::unordered_map<std::string, NPC> npcs;
	std::unordered_map<std::string, std::vector< std::string>> modelAnimationNames;

}