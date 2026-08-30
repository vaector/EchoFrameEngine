#include "Shared.h"
#include "raymath.h"

std::string tile1 = "tile1";//Default texture name

namespace Shared {
	bool editorMode = true;
	std::vector<Wall*> walls;
	int wallHeight = 200;
	std::unordered_map<std::string, Texture2D> textures;
	std::unordered_map<std::string, Model> models;
	std::string* selectedTextureName = &tile1;
	std::string* selectedModelName = nullptr;
	std::unordered_map<int, WorldObject> worldObjects;
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
	Player player;
	Player free;

	int layer = 0;

	bool npcMode = false;
	std::unordered_map<std::string, NPC> npcs;
	NPC* selectedNpc = nullptr;
	std::unordered_map<std::string, std::vector< std::string>> modelAnimationNames;

	WorldObject* selectedWorldObject = nullptr;
	Cutscene cutscenes;
	std::string trackName;
	bool isCutscenePlaying = false;

	std::unordered_map<std::string, Sound> sounds;
	std::unordered_map<std::string, Music> music;

	Vector3 rotationAxis = {0,1,0};

	BoundingBox GetUpdatedBoundingBox(const BoundingBox& original, Vector3 position, Vector3 scale, Vector3 rotationAxis, float rotationAngle)
	{
		Vector3 corners[8] =
		{
			{ original.min.x, original.min.y, original.min.z },
			{ original.max.x, original.min.y, original.min.z },
			{ original.min.x, original.max.y, original.min.z },
			{ original.max.x, original.max.y, original.min.z },
			{ original.min.x, original.min.y, original.max.z },
			{ original.max.x, original.min.y, original.max.z },
			{ original.min.x, original.max.y, original.max.z },
			{ original.max.x, original.max.y, original.max.z },
		};

		BoundingBox box;
		box.min = { FLT_MAX, FLT_MAX, FLT_MAX };
		box.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (int i = 0; i < 8; i++)
		{
			Vector3 scaled = { corners[i].x * scale.x, corners[i].y * scale.y, corners[i].z * scale.z };
			Vector3 rotated = Vector3RotateByAxisAngle(scaled, rotationAxis, DEG2RAD * rotationAngle);
			Vector3 world = Vector3Add(rotated, position);

			box.min.x = fminf(box.min.x, world.x);
			box.min.y = fminf(box.min.y, world.y);
			box.min.z = fminf(box.min.z, world.z);

			box.max.x = fmaxf(box.max.x, world.x);
			box.max.y = fmaxf(box.max.y, world.y);
			box.max.z = fmaxf(box.max.z, world.z);
		}

		return box;
	}
}