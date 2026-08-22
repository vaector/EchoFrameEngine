#include "SaveSystem.h"
#include "raylib.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include "Shared.h"

namespace fs = std::filesystem;

void SaveSystem::SaveGame(const std::string& filename)
{
	//Open Folder
	if (!fs::exists("save")) {
		TraceLog(LOG_FATAL, "ERROR::SAVE_FOLDER_DOES_NOT_EXIST");
		return;
	}
	//Open File
	std::ofstream file("save/" + filename);//Read and Write (ofstream)
	if (!file.is_open())
	{
		TraceLog(LOG_ERROR, "ERROR::COULD_NOT_OPEN_SAVE_FILE");
		return;
	}

	//Create JSON object and save it in the file
	json data;
	SaveWalls(data);
	SaveSectors(data);
	SaveRamps(data);
	SaveDoorSwitches(data);
	SaveWorldObjects(data);
	SaveNPCs(data);
	SaveCutscenes(data);
	SavePlayer(data);

	file << data.dump(4); // Dump the JSON object to the file with indentation of 4 spaces
	file.close();
}

void SaveSystem::LoadGame(const std::string& filename)
{
	//Open Folder
	if (!fs::exists("save")) {
		TraceLog(LOG_FATAL, "ERROR::SAVE_FOLDER_DOES_NOT_EXIST");
		return;
	}

	std::ifstream file("save/" + filename);//Read only (ifstream)

	if (!file.is_open())
	{
		TraceLog(LOG_ERROR, "ERROR::COULD_NOT_OPEN_SAVE_FILE");
		return;
	}

	json data;
	file >> data;

	file.close();

	LoadWalls(data);
	LoadSectors(data);
	LoadRamps(data);
	LoadDoorSwitches(data);
	LoadWorldObjects(data);
	LoadNPCs(data);
	LoadCutscenes(data);
	LoadPlayer(data);
}

void SaveSystem::SaveWalls(json& j)
{
	j["walls"] = json::array();

	for(auto& wall: Shared::walls) {
		json w;
		w["id"] = wall->id;
		w["currentWallId"] = Wall::currentWallId;
		w["startPos"] = { wall->startPos.x, wall->startPos.y };
		w["endPos"] = { wall->endPos.x, wall->endPos.y };
		w["floor"] = wall->floor;
		w["wallHeight"] = wall->wallHeight;
		w["textureName"] = wall->textureName;
		w["inSector"] = wall->inSector;
		w["isDoor"] = wall->isDoor;
		w["isOpen"] = wall->isOpen;
		w["layer"] = wall->layer;
		
		j["walls"].push_back(w);
	}
}
void SaveSystem::LoadWalls(const json& j)
{
	Shared::walls.clear();

	if (!j.contains("walls"))
		return;

	for (const auto& w : j["walls"])
	{
		Wall* wall = new Wall();

		wall->id = w["id"].get<int>();
		Wall::currentWallId = w["currentWallId"].get<int>();

		wall->startPos = {
			w["startPos"][0].get<float>(),
			w["startPos"][1].get<float>()
		};
		wall->endPos = {
			w["endPos"][0].get<float>(),
			w["endPos"][1].get<float>()
		};
		wall->floor = w["floor"].get<float>();
		wall->wallHeight = w["wallHeight"].get<float>();

		wall->textureName = w["textureName"].get<std::string>();

		wall->inSector = w["inSector"].get<bool>();
		wall->isDoor = w["isDoor"].get<bool>();
		wall->isOpen = w["isOpen"].get<bool>();
		wall->layer = w["layer"].get<int>();

		Shared::walls.push_back(wall);
	}
}

void SaveSystem::SaveSectors(json& j)
{
	j["sectors"] = json::array();
	for (auto& sector : Shared::sectors) {
		json s;

		s["floorPoints"] = json::array();
		for(auto& point : sector.floorPoints)
			s["floorPoints"].push_back({ point.x, point.y });

		s["ceilingPoints"] = json::array();
		for(auto& point : sector.ceilingPoints)
			s["ceilingPoints"].push_back({ point.x, point.y });

		s["indices"] = json::array();
		for(auto& index : sector.indices)
			s["indices"].push_back(index);	

		s["floor"] = sector.floor;
		s["floorTextureName"] = sector.floorTextureName;
		s["ceilingTextureName"] = sector.ceilingTextureName;
		s["hasFloor"] = sector.hasFloor;
		s["hasCeiling"] = sector.hasCeiling;
		s["layer"] = sector.layer;

		j["sectors"].push_back(s);
	}
}

void SaveSystem::LoadSectors(const json& j)
{
	Shared::sectors.clear();

	if (!j.contains("sectors"))
		return;

	for (const auto& w : j["sectors"])
	{
		Sector sector;
		for (const auto& point : w["floorPoints"])
			sector.floorPoints.push_back({ point[0].get<float>(), point[1].get<float>() });
		for (const auto& point : w["ceilingPoints"])
			sector.ceilingPoints.push_back({ point[0].get<float>(), point[1].get<float>() });
		for (const auto& index : w["indices"])
			sector.indices.push_back(index.get<int>());
		sector.floor = w["floor"].get<int>();
		sector.floorTextureName = w["floorTextureName"].get<std::string>();
		sector.ceilingTextureName = w["ceilingTextureName"].get<std::string>();
		sector.hasFloor = w["hasFloor"].get<bool>();
		sector.hasCeiling = w["hasCeiling"].get<bool>();
		sector.layer = w["layer"].get<int>();

		
		Shared::sectors.push_back(sector);
	}
}

void SaveSystem::SaveRamps(json& j)
{
	j["ramps"] = json::array();
	for (auto& ramp : Shared::ramps) {
		json r;
		r["startPos"] = { ramp.startPos.x, ramp.startPos.y };
		r["endPos"] = { ramp.endPos.x, ramp.endPos.y };
		r["floor"] = ramp.floor;
		r["rampHeight"] = ramp.rampHeight;
		r["halfWidth"] = ramp.halfWidth;
		r["textureName"] = ramp.textureName;
		r["layer"] = ramp.layer;

		j["ramps"].push_back(r);
	}
}

void SaveSystem::LoadRamps(const json& j)
{
	Shared::ramps.clear();

	if (!j.contains("ramps"))
		return;

	for (auto& r : j["ramps"]) {
		Ramp ramp;
		ramp.startPos = { r["startPos"][0].get<float>(),r["startPos"][1].get<float>() };
		ramp.endPos = { r["endPos"][0].get<float>(),r["endPos"][1].get<float>() };
		ramp.textureName = r["textureName"].get<std::string>();
		
		ramp.floor = r["floor"].get<int>();
		ramp.layer = r["layer"].get<int>();
		ramp.halfWidth = r["halfWidth"].get<int>();
		ramp.rampHeight = r["rampHeight"].get<float>();

		Shared::ramps.push_back(ramp);
	}
}

void SaveSystem::SaveDoorSwitches(json& j)
{
	j["doorSwitches"] = json::array();
	for (auto& doorSwitch : Shared::doorSwitches) {
		json d;
		d["pos"] = { doorSwitch.pos.x, doorSwitch.pos.y, doorSwitch.pos.z };
		d["floor"] = doorSwitch.floor;
		d["textureName"] = doorSwitch.textureName;
		d["layer"] = doorSwitch.layer;
		d["wallId"] = doorSwitch.wallId;

		j["doorSwitches"].push_back(d);
	}
}

void SaveSystem::LoadDoorSwitches(json& j)
{
	Shared::doorSwitches.clear();

	if (!j.contains("doorSwitches"))
		return;
	for (auto& d : j["doorSwitches"]) {
		DoorSwitch ds;
		ds.pos = { d["pos"][0].get<float>(), d["pos"][1].get<float>() , d["pos"][2].get<float>() };
		ds.floor = d["floor"].get<int>();
		ds.textureName = d["textureName"].get<std::string>();
		
		ds.wallId = d["wallId"].get<int>();
		
		ds.linkedWall = nullptr;
		for(auto& wall: Shared::walls){
			if (wall->id == ds.wallId)
				ds.linkedWall= wall;
		}

		ds.layer = d["layer"].get<int>();

		Shared::doorSwitches.push_back(ds);
	}
}

void SaveSystem::SaveWorldObjects(json& j)
{
	j["worldObjects"] = json::array();
	for(auto& [id, worldObject] : Shared::worldObjects) {
		json w;

		w["id"] = id;
		w["currentId"] = WorldObject::currentId;

		w["modelName"] = worldObject.modelName;
		w["position"] = { worldObject.position.x, worldObject.position.y, worldObject.position.z };
		w["scale"] = { worldObject.scale.x, worldObject.scale.y, worldObject.scale.z };
		w["rotationAxis"] = { worldObject.rotationAxis.x, worldObject.rotationAxis.y, worldObject.rotationAxis.z };
		w["rotationAngle"] = worldObject.rotationAngle;
		w["collider"] = { { worldObject.collider.min.x, worldObject.collider.min.y, worldObject.collider.min.z },{ worldObject.collider.max.x, worldObject.collider.max.y, worldObject.collider.max.z } };
		w["originalCollider"] = { { worldObject.originalCollider.min.x, worldObject.originalCollider.min.y, worldObject.originalCollider.min.z },{ worldObject.originalCollider.max.x, worldObject.originalCollider.max.y, worldObject.originalCollider.max.z } };

		j["worldObjects"].push_back(w);
	}
}

void SaveSystem::LoadWorldObjects(json& j)
{
	Shared::worldObjects.clear();
	if (!j.contains("worldObjects"))
		return;
	
	for (auto& w : j["worldObjects"]) {
		WorldObject worldObject;

		worldObject.id = w["id"].get<int>();
		WorldObject::currentId = w["currentId"].get<int>();

		worldObject.modelName = w["modelName"].get<std::string>();
		worldObject.position = { w["position"][0].get<float>(), w["position"][1].get<float>(), w["position"][2].get<float>() };
		worldObject.scale = { w["scale"][0].get<float>(), w["scale"][1].get<float>(), w["scale"][2].get<float>() };
		worldObject.rotationAxis = { w["rotationAxis"][0].get<float>(), w["rotationAxis"][1].get<float>(), w["rotationAxis"][2].get<float>() };
		worldObject.rotationAngle = w["rotationAngle"].get<float>();
		worldObject.collider = { {w["collider"][0][0], w["collider"][0][1], w["collider"][0][2]}, {w["collider"][1][0], w["collider"][1][1], w["collider"][1][2]} };
		worldObject.originalCollider = { {w["originalCollider"][0][0], w["originalCollider"][0][1], w["originalCollider"][0][2]}, {w["originalCollider"][1][0], w["originalCollider"][1][1], w["originalCollider"][1][2]} };
	
		Shared::worldObjects[worldObject.id] = worldObject;
	}
}

void SaveSystem::SaveNPCs(json& j)
{
	j["npcs"] = json::array();
	for(auto& [name, npc] : Shared::npcs) {
		json n;

		//Entity Stuff
		n["modelName"] = npc.modelName;
		n["position"] = { npc.position.x, npc.position.y, npc.position.z };
		n["scale"] = { npc.scale.x, npc.scale.y, npc.scale.z };
		n["rotationAxis"] = { npc.rotationAxis.x, npc.rotationAxis.y, npc.rotationAxis.z };
		n["rotationAngle"] = npc.rotationAngle;
		n["collider"] = { { npc.collider.min.x, npc.collider.min.y, npc.collider.min.z },{ npc.collider.max.x, npc.collider.max.y, npc.collider.max.z } };
		n["originalCollider"] = { { npc.originalCollider.min.x, npc.originalCollider.min.y, npc.originalCollider.min.z },{ npc.originalCollider.max.x, npc.originalCollider.max.y, npc.originalCollider.max.z } };

		//NPC Stuff
		n["name"] = npc.name;//Model and modelANimation is derived from the npc name.
		n["animSpeed"] = npc.animSpeed;
		n["currentAnim"] = npc.currentAnim;
		n["isPlayingMap"] = npc.isPlayingMap;
		n["isFinishedMap"] = npc.isFinishedMap;
		n["currentAccumulatorMap"] = npc.currentAccumulatorMap;
		n["currentFrameMap"] = npc.currentFrameMap;


		j["npcs"].push_back(n);
	}
}

void SaveSystem::LoadNPCs(json& j)
{
	Shared::npcs.clear();
	if (!j.contains("npcs"))
		return;

	for (auto& n : j["npcs"]) {
		
		NPC npc;

		//Entity stuff
		npc.modelName = n["modelName"].get<std::string>();
		npc.position = { n["position"][0].get<float>(), n["position"][1].get<float>(), n["position"][2].get<float>() };
		npc.scale = { n["scale"][0].get<float>(), n["scale"][1].get<float>(), n["scale"][2].get<float>() };
		npc.rotationAxis = { n["rotationAxis"][0].get<float>(), n["rotationAxis"][1].get<float>(), n["rotationAxis"][2].get<float>() };
		npc.rotationAngle = n["rotationAngle"].get<float>();
		npc.collider = { {n["collider"][0][0], n["collider"][0][1], n["collider"][0][2]}, {n["collider"][1][0], n["collider"][1][1], n["collider"][1][2]} };
		npc.originalCollider = { {n["originalCollider"][0][0], n["originalCollider"][0][1], n["originalCollider"][0][2]}, {n["originalCollider"][1][0], n["originalCollider"][1][1], n["originalCollider"][1][2]} };

		//Npc stuff
		npc.name = n["name"].get<std::string>();
		npc.animSpeed = n["animSpeed"].get<float>();
		npc.currentAnim = n["currentAnim"].get<int>();
		npc.isPlayingMap = n["isPlayingMap"].get<std::unordered_map<int, bool>>();
		npc.isFinishedMap = n["isFinishedMap"].get<std::unordered_map<int, bool>>();
		npc.currentAccumulatorMap = n["currentAccumulatorMap"].get<std::unordered_map<int, float>>();
		npc.currentFrameMap = n["currentFrameMap"].get<std::unordered_map<int, int>>();

		npc.model = Shared::models[npc.name];

		std::string path = "models/";
		path += npc.name;
		path += ".glb";
		npc.modelAnim = LoadModelAnimations(path.c_str(), &npc.animCount);

		Shared::npcs[npc.name] = npc;
	}
}

void SaveSystem::SaveCutscenes(json& j)
{
	j["cutscenes"] = json::array();

	//World Object
	for (auto& [id, tracks] : Shared::cutscenes.worldObjectCutscene) {
		json c;

		c["worldObjectId"] = id;
		c["tracks"] = json::array();

		for (auto& t : tracks) {
			json track;
			track["time"] = t.time;
			track["currentFrame"] = t.currentFrame;
			track["name"] = t.name;
			track["worldObjectId"] = t.worldObjectID;
			track["npcName"] = t.npcName;
			track["animSpeed"] = t.animSpeed;

			track["keyframes"] = json::array();
			for (auto& key : t.keyframes) {
				json k;
				k["frame"] = key.frame;
				k["position"] = { key.position.x, key.position.y, key.position.z };
				k["scale"] = { key.scale.x, key.scale.y, key.scale.z };
				k["rotationAxis"] = { key.rotationAxis.x, key.rotationAxis.y, key.rotationAxis.z };
				k["rotationAngle"] = key.rotationAngle;

				track["keyframes"].push_back(k);
			}

			json okf;
			okf["frame"] = t.originalKeyFrame.frame;
			okf["position"] = { t.originalKeyFrame.position.x, t.originalKeyFrame.position.y, t.originalKeyFrame.position.z };
			okf["scale"] = { t.originalKeyFrame.scale.x, t.originalKeyFrame.scale.y, t.originalKeyFrame.scale.z };
			okf["rotationAxis"] = { t.originalKeyFrame.rotationAxis.x, t.originalKeyFrame.rotationAxis.y, t.originalKeyFrame.rotationAxis.z };
			okf["rotationAngle"] = t.originalKeyFrame.rotationAngle;

			track["originalKeyFrame"] = okf;

			c["tracks"].push_back(track);
		}

		j["cutscenes"].push_back(c);

	}

	//Npc

}

void SaveSystem::LoadCutscenes(json& j)
{
	Shared::cutscenes.worldObjectCutscene.clear();
	Shared::cutscenes.npcCutscene.clear();

	if (!j.contains("cutscenes"))
		return;

	for (auto& c : j["cutscenes"]) {

		int worldObjectId = c["worldObjectId"].get<int>();
		std::vector<Track> tracks;

		for (auto& t : c["tracks"]) {
			Track track;

			track.time = t["time"].get<float>();
			track.currentFrame = t["currentFrame"].get<int>();
			track.name = t["name"].get<std::string>();
			track.worldObjectID = t["worldObjectId"].get<int>();
			track.npcName = t["npcName"].get<std::string>();
			track.animSpeed = t["animSpeed"].get<float>();

			for (auto& key : t["keyframes"]) {
				KeyFrame k;

				k.frame = key["frame"].get<int>();
				k.position = { key["position"][0].get<float>(), key["position"][1].get<float>(), key["position"][2].get<float>() };
				k.scale = { key["scale"][0].get<float>(), key["scale"][1].get<float>(), key["scale"][2].get<float>() };
				k.rotationAxis = { key["rotationAxis"][0].get<float>(), key["rotationAxis"][1].get<float>(), key["rotationAxis"][2].get<float>() };
				k.rotationAngle = key["rotationAngle"].get<float>();

				track.keyframes.push_back(k);
			}

			auto& okf = t["originalKeyFrame"];
			track.originalKeyFrame.frame = okf["frame"].get<int>();
			track.originalKeyFrame.position = { okf["position"][0].get<float>(), okf["position"][1].get<float>(), okf["position"][2].get<float>() };
			track.originalKeyFrame.scale = { okf["scale"][0].get<float>(), okf["scale"][1].get<float>(), okf["scale"][2].get<float>() };
			track.originalKeyFrame.rotationAxis = { okf["rotationAxis"][0].get<float>(), okf["rotationAxis"][1].get<float>(), okf["rotationAxis"][2].get<float>() };
			track.originalKeyFrame.rotationAngle = okf["rotationAngle"].get<float>();

			tracks.push_back(track);
		}

		Shared::cutscenes.worldObjectCutscene[worldObjectId] = tracks;
	}

	// NPC cutscene loading goes here
}

void SaveSystem::SavePlayer(json& j)
{
	j["player"] = json::object();
	json player;
	player["moveSpeed"] = Shared::player.moveSpeed;
	player["gravity"] = Shared::player.gravity;
	player["jump"] = Shared::player.jump;
	player["height"] = Shared::player.height;
	player["radius"] = Shared::player.radius;

	player["setPlayerPos"] = Shared::setPlayerPos;
	player["playerPos"] = { Shared::playerPos.x, Shared::playerPos.y, Shared::playerPos.z };

	j["player"] = player;
}

void SaveSystem::LoadPlayer(json& j)
{
	if (!j.contains("player"))
		return;

	auto& p = j["player"];

	Shared::player.moveSpeed = p["moveSpeed"].get<int>();
	Shared::player.gravity = p["gravity"].get<float>();
	Shared::player.jump = p["jump"].get<float>();
	Shared::player.height = p["height"].get<float>();
	Shared::player.radius = p["radius"].get<float>();

	Shared::setPlayerPos = p["setPlayerPos"].get<bool>();
	Shared::playerPos = { p["playerPos"][0].get<float>(), p["playerPos"][1].get<float>() , p["playerPos"][2].get<float>() };

}

