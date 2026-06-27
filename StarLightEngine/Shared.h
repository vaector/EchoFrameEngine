#pragma once
#include "TileEditor.h"
#include <unordered_map>
#include <string>


namespace Shared {
	extern std::vector<Wall> walls;
	extern int wallHeight;
	extern std::unordered_map<std::string, Texture2D> textures;
	extern std::vector<Sector> sectors;
	extern Texture2D* selectedTexture;
	extern bool sectorMode;
}

//-------WALLS VECTOR------> A vector of walls(wall-> {{start position, end position}, floor, texture}) -> SHARED.H (Shared::walls)
//-------SECTORS VECTOR------> A vector of sectors(sector->{vecotor ot points, floor, texture}) -> SHARED.H (Shared::sectors)
//-------TEXTURES MAP------> A map of textures(key-> filename, val->Texture2D) -> SHARED.H (Shared::sectors)
//-------SELECTEDTEXTURE---> A pointer pointing at the selected texture through the tilemap editor imgui window. -> SHARED.H (Shared::sectors)
//-------SECTORMODE--------> bool value. -> true means consider new walls for a sector | false means do not consider(no sectors)