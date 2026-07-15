#pragma once
#include "TileEditor.h"
#include <unordered_map>
#include <string>
#include "Ramp.h"

namespace Shared {
	extern std::vector<Wall*> walls;
	extern int wallHeight;
	extern std::unordered_map<std::string, Texture2D> textures;
	extern std::vector<Sector> sectors;
	extern std::vector<Ramp> ramps;
	extern Texture2D* selectedTexture;
	extern bool sectorMode;
	extern bool floorOnly;
	extern bool ceilingOnly;
	extern int floor;
	extern bool onImguiWindow;
	extern bool doorMode;
	extern std::vector<DoorSwitch> doorSwitches;
	extern Vector3 playerPos;
	extern bool setPlayerPos;
}

//-------WALLS VECTOR------> A vector of walls(wall-> {{start position, end position}, floor, texture}) -> SHARED.H (Shared::walls)
//-------SECTORS VECTOR------> A vector of sectors(sector->{vecotor ot points, floor, texture}) -> SHARED.H (Shared::sectors)
//-------TEXTURES MAP------> A map of textures(key-> filename, val->Texture2D) -> SHARED.H (Shared::sectors)
//-------SELECTEDTEXTURE---> A pointer pointing at the selected texture through the tilemap editor imgui window. -> SHARED.H (Shared::sectors)
//-------SECTORMODE--------> bool value. -> true means consider new walls for a sector | false means do not consider(no sectors)
//-------FLOOR-------------> The floor currently on-> updated in ImguiWindow.h.
//-------RAMPMODE----------> bool value that tells us if we are adding ramps or not.(ImguiWindow.h)
//-------onImguiWindow-----> bool value that tells us if the cursor is on an IMGUI window/Item(ImguiWindow.h)