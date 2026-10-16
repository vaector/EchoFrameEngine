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
	SaveLights(data);
	SaveDecals(data);
	SaveTriggers(data);

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

	//Clear pointers and vectors/arrays/maps
	Shared::selectedWorldObject = nullptr;
	Shared::selectedNpc = nullptr;

	LoadWalls(data);
	LoadSectors(data);
	LoadRamps(data);
	LoadDoorSwitches(data);
	LoadWorldObjects(data);
	LoadNPCs(data);
	LoadCutscenes(data);
	LoadPlayer(data);
	LoadLights(data);
	LoadDecals(data);
	LoadTriggers(data);
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

		//Entity stuff
		w["modelName"] = worldObject.modelName;
		w["position"] = { worldObject.position.x, worldObject.position.y, worldObject.position.z };
		w["scale"] = { worldObject.scale.x, worldObject.scale.y, worldObject.scale.z };
		w["rotationAxis"] = { worldObject.rotationAxis.x, worldObject.rotationAxis.y, worldObject.rotationAxis.z };
		w["rotationAngle"] = worldObject.rotationAngle;

		w["collisionBox"] = json::array();
		for (int i = 0; i < worldObject.collisionBoxes.size(); i++) {
			auto& box = worldObject.collisionBoxes[i];
			w["collisionBox"].push_back({ { box.min.x, box.min.y, box.min.z }, {box.max.x, box.max.y, box.max.z} });
		}

		w["originalBox"] = json::array();
		for (int i = 0; i < worldObject.originalBoxes.size(); i++) {
			auto& box = worldObject.originalBoxes[i];
			w["originalBox"].push_back({ { box.min.x, box.min.y, box.min.z }, {box.max.x, box.max.y, box.max.z} });
		}

		//World Object Stuff

		w["id"] = id;
		w["currentId"] = WorldObject::currentId;
		w["isMeshInstance"] = worldObject.isMeshInstance;


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
		worldObject.isMeshInstance = w["isMeshInstance"].get<bool>();

		worldObject.modelName = w["modelName"].get<std::string>();
		worldObject.position = { w["position"][0].get<float>(), w["position"][1].get<float>(), w["position"][2].get<float>() };
		worldObject.scale = { w["scale"][0].get<float>(), w["scale"][1].get<float>(), w["scale"][2].get<float>() };
		worldObject.rotationAxis = { w["rotationAxis"][0].get<float>(), w["rotationAxis"][1].get<float>(), w["rotationAxis"][2].get<float>() };
		worldObject.rotationAngle = w["rotationAngle"].get<float>();

		//Collision Boxes
		for (auto& box : w["collisionBox"]) {
			worldObject.collisionBoxes.push_back({ { box[0][0].get<float>(), box[0][1].get<float>(), box[0][2].get<float>() }, 
				{ box[1][0].get<float>(), box[1][1].get<float>(), box[1][2].get<float>() } });
		}

		//Original Boxes
		for (auto& box : w["originalBox"]) {
			worldObject.originalBoxes.push_back({ { box[0][0].get<float>(), box[0][1].get<float>(), box[0][2].get<float>() },
				{ box[1][0].get<float>(), box[1][1].get<float>(), box[1][2].get<float>() } });
		}

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
		
		n["collisionBox"] = json::array();
		for (int i = 0; i < npc.collisionBoxes.size(); i++) {
			auto& box = npc.collisionBoxes[i];
			n["collisionBox"].push_back({ { box.min.x, box.min.y, box.min.z }, {box.max.x, box.max.y, box.max.z} });
		}

		n["originalBox"] = json::array();
		for (int i = 0; i < npc.originalBoxes.size(); i++) {
			auto& box = npc.originalBoxes[i];
			n["originalBox"].push_back({ { box.min.x, box.min.y, box.min.z }, {box.max.x, box.max.y, box.max.z} });
		}

		
		
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
		
		//Collision Boxes
		for (auto& box : n["collisionBox"]) {
			npc.collisionBoxes.push_back({ { box[0][0].get<float>(), box[0][1].get<float>(), box[0][2].get<float>() },
				{ box[1][0].get<float>(), box[1][1].get<float>(), box[1][2].get<float>() } });
		}

		//Original Boxes
		for (auto& box : n["originalBox"]) {
			npc.originalBoxes.push_back({ { box[0][0].get<float>(), box[0][1].get<float>(), box[0][2].get<float>() },
				{ box[1][0].get<float>(), box[1][1].get<float>(), box[1][2].get<float>() } });
		}


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
	
	//World Object
	j["worldObjectCutscenes"] = json::array();

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
			track["animSpeed"] = t.animDuration;
			track["finished"] = t.finished;

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

		j["worldObjectCutscenes"].push_back(c);

	}

	//Playing Track/IsCutscenePlaying
	j["playingTrack"] = Shared::cutscenes.playingTrack;
	j["isCutscenePlaying"] = Shared::isCutscenePlaying;


	//Npc
	j["npcCutscenes"] = json::array();

	for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
		json c;

		c["npcName"] = name;
		c["tracks"] = json::array();

		for (auto& t : tracks) {
			json track;
			track["time"] = t.time;
			track["currentFrame"] = t.currentFrame;
			track["name"] = t.name;
			track["worldObjectId"] = t.worldObjectID;
			track["npcName"] = t.npcName;
			track["animSpeed"] = t.animDuration;
			track["finished"] = t.finished;

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

		j["npcCutscenes"].push_back(c);

	}

}

void SaveSystem::LoadCutscenes(json& j)
{
	Shared::cutscenes.worldObjectCutscene.clear();
	Shared::cutscenes.npcCutscene.clear();

	if (j.contains("worldObjectCutscenes")) {

		//World Object
		for (auto& c : j["worldObjectCutscenes"]) {

			int worldObjectId = c["worldObjectId"].get<int>();
			std::vector<Track> tracks;

			for (auto& t : c["tracks"]) {
				Track track;

				track.time = t["time"].get<float>();
				track.currentFrame = t["currentFrame"].get<int>();
				track.name = t["name"].get<std::string>();
				track.worldObjectID = t["worldObjectId"].get<int>();
				track.npcName = t["npcName"].get<std::string>();
				track.animDuration = t["animSpeed"].get<float>();
				track.finished = t["finished"].get<bool>();

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
	}

	//PLayingTrack/IsCutscenePlaying
	if (j.contains("playingTrack")) 
		Shared::cutscenes.playingTrack = j["playingTrack"].get<std::string>();
	if (j.contains("isCutscenePlaying"))
		Shared::isCutscenePlaying = j["isCutscenePlaying"].get<bool>();

	// NPC cutscene loading goes here
	if (j.contains("npcCutscenes")) {
		
		for (auto& c : j["npcCutscenes"]) {

			std::string npcName = c["npcName"].get<std::string>();
			std::vector<Track> tracks;

			for (auto& t : c["tracks"]) {
				Track track;

				track.time = t["time"].get<float>();
				track.currentFrame = t["currentFrame"].get<int>();
				track.name = t["name"].get<std::string>();
				track.worldObjectID = t["worldObjectId"].get<int>();
				track.npcName = t["npcName"].get<std::string>();
				track.animDuration = t["animSpeed"].get<float>();
				track.finished = t["finished"].get<bool>();

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

			Shared::cutscenes.npcCutscene[npcName] = tracks;
		}
	}
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

void SaveSystem::SaveLights(json& j)
{
	json lights = json::array();

	//Default Light
	json defaultLight;

	defaultLight["position"] = { LightSystem::defaultLight.position.x, LightSystem::defaultLight.position.y, LightSystem::defaultLight.position.z };
	defaultLight["dir"] = { LightSystem::defaultLight.dir.x, LightSystem::defaultLight.dir.y, LightSystem::defaultLight.dir.z };
	defaultLight["color"] = { LightSystem::defaultLight.color.r, LightSystem::defaultLight.color.g, LightSystem::defaultLight.color.b, LightSystem::defaultLight.color.a };
	defaultLight["intensity"] = LightSystem::defaultLight.intensity;
	defaultLight["enabled"] = LightSystem::defaultLight.enabled;
	defaultLight["type"] = LightSystem::defaultLight.type;
	defaultLight["range"] = LightSystem::defaultLight.range;
	defaultLight["spotAngle"] = LightSystem::defaultLight.spotAngle;

	lights.push_back(defaultLight);

	for (const auto& light : Shared::lights) {
		json lightJson;
		lightJson["position"] = { light.position.x, light.position.y, light.position.z };
		lightJson["dir"] = { light.dir.x, light.dir.y, light.dir.z };
		lightJson["color"] = { light.color.r, light.color.g, light.color.b, light.color.a };
		lightJson["intensity"] = light.intensity;
		lightJson["enabled"] = light.enabled;
		lightJson["type"] = light.type;
		lightJson["range"] = light.range;	
		lightJson["spotAngle"] = light.spotAngle;

		lights.push_back(lightJson);
	}

	j["lights"] = lights;
}

void SaveSystem::LoadLights(const json& j)
{
	if (!j.contains("lights"))
		return;

	Shared::lights.clear();

	//Default Light
	auto& defaultLight = j["lights"][0];

	Light light;
	light.position = { defaultLight["position"][0].get<float>(), defaultLight["position"][1].get<float>(),defaultLight["position"][2].get<float>() };
	light.dir = { defaultLight["dir"][0].get<float>(), defaultLight["dir"][1].get<float>(),defaultLight["dir"][2].get<float>() };
	light.color = { defaultLight["color"][0].get<unsigned char>(), defaultLight["color"][1].get<unsigned char>(),defaultLight["color"][2].get<unsigned char>(),defaultLight["color"][3].get<unsigned char>() };
	light.type = static_cast<LIGHT_TYPE>(defaultLight["type"].get<int>());
	light.intensity = defaultLight["intensity"].get<float>();
	light.range = defaultLight["range"].get<float>();
	light.spotAngle = defaultLight["spotAngle"].get<float>();
	light.enabled = defaultLight["enabled"].get<bool>();

	Shared::lights.push_back(light);


	for(auto& l : j["lights"])
	{
		Light light;
		light.position = { l["position"][0].get<float>(), l["position"][1].get<float>(),l["position"] [2] .get<float>() };
		light.dir = { l["dir"][0].get<float>(), l["dir"][1].get<float>(),l["dir"][2].get<float>() };
		light.color = { l["color"][0].get<unsigned char>(), l["color"][1].get<unsigned char>(),l["color"][2].get<unsigned char>(),l["color"][3].get<unsigned char>() };
		light.type = static_cast<LIGHT_TYPE>(l["type"].get<int>());
		light.intensity = l["intensity"].get<float>();
		light.range = l["range"].get<float>();
		light.spotAngle = l["spotAngle"].get<float>();
		light.enabled = l["enabled"].get<bool>();

		Shared::lights.push_back(light);
	}
}

void SaveSystem::SaveDecals(json& j)
{
	json decals = json::array();

	for (const auto& d : Shared::decals) {
		json decalJson;
		decalJson["position"] = { d.pos.x, d.pos.y, d.pos.z };
		decalJson["scale"] = d.scale;
		decalJson["floor"] = d.floor;
		decalJson["textureName"] = d.textureName;
		decalJson["wallDir"] = { d.wallDir.x, d.wallDir.y };

		decals.push_back(decalJson);
	}

	j["decals"] = decals;
}

void SaveSystem::LoadDecals(const json& j)
{
	if (!j.contains("decals"))
		return;

	Shared::decals.clear();

	for (auto& d : j["decals"])
	{
		Decal decal;
		decal.pos = { d["position"][0].get<float>(), d["position"][1].get<float>(), d["position"][2].get<float>() };
		decal.scale = d["scale"].get<float>();
		decal.floor = d["floor"].get<int>();
		decal.textureName = d["textureName"].get<std::string>();
		decal.wallDir = { d["wallDir"][0].get<float>(), d["wallDir"][1].get<float>() };

		Shared::decals.push_back(decal);
	}
}

void SaveSystem::SaveTriggers(json& j)
{
	json boxTriggers = json::array();
	//Box Triggers
	for (const auto& [name, t] : Shared::boxTriggers) {
		json triggerJson;
		triggerJson["position"] = { t.pos.x, t.pos.y, t.pos.z };
		triggerJson["size"] = { t.size.x, t.size.y, t.size.z };
		triggerJson["name"] = name;
		triggerJson["box"] = { {t.box.min.x, t.box.min.y, t.box.min.z}, {t.box.max.x, t.box.max.y, t.box.max.z} };
		boxTriggers.push_back(triggerJson);
	}
	j["boxTriggers"] = boxTriggers;
}

void SaveSystem::LoadTriggers(const json& j)
{

	//Box Triggers
	if (!j.contains("boxTriggers"))
		return;
	Shared::boxTriggers.clear();

	for (auto& t : j["boxTriggers"])
	{
		BoxTrigger trigger;
		trigger.pos = { t["position"][0].get<float>(), t["position"][1].get<float>(), t["position"][2].get<float>() };
		trigger.size = { t["size"][0].get<float>(), t["size"][1].get<float>(), t["size"][2].get<float>() };
		trigger.name = t["name"].get<std::string>();
		trigger.box = { {t["box"][0][0].get<float>(), t["box"][0][1].get<float>(), t["box"][0][2].get<float>()}, {t["box"][1][0].get<float>(), t["box"][1][1].get<float>(), t["box"][1][2].get<float>()} };

		Shared::boxTriggers[trigger.name] = trigger;
	}
}
