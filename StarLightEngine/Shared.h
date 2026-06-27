#pragma once
#include "TileEditor.h"

#include <unordered_map>
#include <string>


namespace Shared {
	extern std::vector<Wall> walls;
	extern int wallHeight;
	extern std::unordered_map<std::string, Texture2D> textures;
	extern Texture2D* selectedTexture;
}

//-------WALLS VECTOR------> A vector of walls(wall-> {start position, end position}) -> SHARED.H (Shared::walls)
//-------TEXTURES MAP------> A map of textures(key-> filename, val->Texture2D)
//-------SELECTEDTEXTURE---> A pointer pointing at the selected texture through the tilemap editor imgui window.