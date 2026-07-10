#include "Shared.h"


namespace Shared {
	std::vector<Wall*> walls;
	int wallHeight = 128;
	std::unordered_map<std::string, Texture2D> textures;
	Texture2D* selectedTexture = &textures["tile1"];
	std::vector<Sector> sectors;
	bool sectorMode = true;
	int floor = 0;
	bool onImguiWindow = false;
	std::vector<Ramp> ramps;
	float selectedRampHeight = 0.0f;
	bool doorMode = false;
	std::vector<DoorSwitch> doorSwitches;
	Vector3 playerPos = { 1024,0,1024};
	bool setPlayerPos = false;
}