#include "Shared.h"


namespace Shared {
	std::vector<Wall> walls;
	int wallHeight = 128;
	std::unordered_map<std::string, Texture2D> textures;
	Texture2D* selectedTexture = &textures["tile1"];
	std::vector<Sector> sectors;

	bool sectorMode = false;
	bool rampMode = false;
	int floor = 0;
	bool onImguiWindow = false;

	std::vector<Ramp> ramps;
	float selectedRampHeight = 0.0f;
	
}