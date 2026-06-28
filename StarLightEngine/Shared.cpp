#include "Shared.h"


namespace Shared {
	std::vector<Wall> walls;
	int wallHeight = 128;
	std::unordered_map<std::string, Texture2D> textures;
	Texture2D* selectedTexture = &textures["tile1"];
	std::vector<Sector> sectors;
	bool sectorMode = false;
	int floor = 0;
}