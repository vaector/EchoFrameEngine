#include "ImguiWindow.h"
#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"
#include "Shared.h"
#include "imgui/imgui_stdlib.h"
#include "Entity.h"
#include <iostream>
#include "SaveSystem.h"
#include "LightSystem.h"
#include "DialougeSystem.h"

void ImguiWindow::UpdateImguiTileEditor()
{

	//Set initial values of the LayerNameMap
	if (first) {
		layerNameMap[0] = "Walls";
		layerNameMap[1] = "Sectors";
		layerNameMap[2] = "Objects";
	}

	ImGui::SetNextWindowSizeConstraints(
		ImVec2(200, 200),
		ImVec2(600, 800)
	);

	//------TileMap Editor Window------
	ImGui::Begin("TileMap Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowPos({ 0,0 });


	ImGui::NewLine();

	static std::string fileName = "savegame";
	ImGui::InputText("File Name", &fileName);

	ImGui::NewLine();
	// Save
	if (ImGui::Button("Save"))
		ImGui::OpenPopup("Save?");

	if (ImGui::BeginPopupModal("Save?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to overwrite the save?");

		if (ImGui::Button("Yes"))
		{
			SaveSystem::SaveGame(fileName + ".json");
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("No"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	// Load
	if (ImGui::Button("Load"))
		ImGui::OpenPopup("Load?");

	if (ImGui::BeginPopupModal("Load?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure? Current data will be lost.");

		if (ImGui::Button("Yes"))
		{
			SaveSystem::LoadGame(fileName + ".json");
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("No"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
	ImGui::NewLine();

	//Drop Down box for choosing textures
	static std::string selectedT = "tile1";
	int count = 0;
	ImGui::Text(selectedT.c_str());
	ImGui::NewLine();
	for (auto& [key, tex] : Shared::textures)
	{
		if (ImGui::ImageButton(
			key.c_str(),
			(ImTextureID)(intptr_t)tex.id,
			ImVec2(64, 64)))
		{
			selectedT = key;
			Shared::selectedTextureName = &selectedT;
		}

		if (++count % 4 != 0)
			ImGui::SameLine();
	}

	//Checkbox for sectorMode
	ImGui::NewLine();
	ImGui::Checkbox("Sector Mode", &Shared::sectorMode);
	ImGui::Checkbox("Floor Only", &Shared::floorOnly);
	ImGui::Checkbox("Ceiling Only", &Shared::ceilingOnly);
	ImGui::NewLine();
	//Current Floor
	if (ImGui::InputInt("Floor", &Shared::floor)) {
		if (Shared::floor <= 0)
			Shared::floor = 0;
	}
	ImGui::NewLine();
	//Current Layer
	ImGui::InputText("Layer Name: ", &layerNameMap[Shared::layer]);

	if (ImGui::InputInt("Layer", &Shared::layer)) {
		if (Shared::layer <= 0)
			Shared::layer = 0;
	}
	ImGui::NewLine();

	//Door Mode
	ImGui::Checkbox("Door Mode", &Shared::doorMode);

	ImGui::NewLine();

	//NPC mode
	ImGui::Checkbox("NPC Mode", &Shared::npcMode);


	ImGui::End();


	ImGui::SetNextWindowSizeConstraints(
		ImVec2(200, 400),
		ImVec2(600, 800)
	);

	if (!Shared::editorMode) {

		//------3D Models Window------
		ImGui::Begin("3D Models", nullptr, ImGuiChildFlags_AlwaysAutoResize);
		ImGui::SetWindowPos({ (float)GetScreenWidth() - 600,0 });


		//--PREOPERTIES SECTION
		ImGui::SeparatorText("WorldObject/NPC Properties");
		//World Object
		if (Shared::selectedWorldObject) {
			ImGui::Text("World Object Properties");
			ImGui::NewLine();
			ImGui::Text(("ID: " + std::to_string(Shared::selectedWorldObject->id)).c_str());
			ImGui::Text(("Model Name: " + Shared::selectedWorldObject->modelName).c_str());
			ImGui::NewLine();
			ImGui::Text("Position");
			ImGui::InputFloat3("##Position", &Shared::selectedWorldObject->position.x);
			ImGui::NewLine();
			ImGui::Text("Scale");
			ImGui::InputFloat3("##Scale", &Shared::selectedWorldObject->scale.x);
			ImGui::NewLine();
			ImGui::Text("Rotation");
			ImGui::InputFloat3("##Rotation", &Shared::selectedWorldObject->rotationAxis.x);
		}

		//Npc
		if (Shared::selectedNpc) {
			ImGui::Text("NPC Properties");
			ImGui::NewLine();
			ImGui::Text(("Name: " + Shared::selectedNpc->name).c_str());
			ImGui::NewLine();
			ImGui::Text("Position");
			ImGui::InputFloat3("##Position", &Shared::selectedNpc->position.x);
			ImGui::NewLine();
			ImGui::Text("Scale");
			ImGui::InputFloat3("##Scale", &Shared::selectedNpc->scale.x);
			ImGui::NewLine();
			ImGui::Text("Rotation");
			ImGui::InputFloat3("##Rotation", &Shared::selectedNpc->rotationAxis.x);
		}
		
		//Rotation Axis
		ImGui::SeparatorText("Rotation Axis");

		static bool isAxisX = false;
		static bool isAxisY = true;
		static bool isAxisZ = false;

		ImGui::Checkbox("X", &isAxisX);
		ImGui::SameLine();
		ImGui::Checkbox("Y", &isAxisY);
		ImGui::SameLine();
		ImGui::Checkbox("Z", &isAxisZ);

		Shared::rotationAxis = { (float)isAxisX, (float)isAxisY, (float)isAxisZ };

		ImGui::SeparatorText("3D Models");

		//Search for models
		static std::string search;
		static std::string selectedModel;

		ImGui::InputText("Search", &search);
		ImGui::NewLine();

		if (ImGui::BeginListBox("Models"))
		{
			for (auto& [name, model] : Shared::models)
			{
				if (!search.empty() && name.find(search) == std::string::npos)
					continue;

				bool selected = (selectedModel == name);

				if (ImGui::Selectable(name.c_str(), selected))
				{
					selectedModel = name;
					Shared::selectedModelName = &selectedModel;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndListBox();
		}
		ImGui::NewLine();

		//Animations of selested model
		if (Shared::selectedModelName != nullptr) {

			for (auto& [name, animNames] : Shared::modelAnimationNames) {
				if (name == *Shared::selectedModelName) {
					for (auto& name: animNames) {
						ImGui::Text(name.c_str());
						ImGui::NewLine();
					}
				}
			}

		}

		//--------CUTSCENES-----------
		ImGui::SeparatorText("Cutscenes Section");

		//Add Track
		static Track* newTrack = nullptr;
		
		if (Shared::selectedWorldObject || Shared::selectedNpc && !newTrack) {
			if (ImGui::Button("Add Track")) {

				newTrack = new Track();
				//Add the world object id to the track
				if (Shared::selectedWorldObject) {
					newTrack->worldObjectID = Shared::selectedWorldObject->id;
				}
				//Add the npc name to the track
				if (Shared::selectedNpc) {
					newTrack->npcName = Shared::selectedNpc->name;
				}
				ImGui::OpenPopup("TrackName?");
				Shared::popupOpen = true;
			}
		}
		if (newTrack != nullptr) {
			TrackNamePopup(newTrack->name);
			
		}
		ImGui::NewLine();

		//Add Keyframes
		if ((Shared::selectedWorldObject || Shared::selectedNpc) && newTrack) {
			if (ImGui::Button("Add KeyFrame")) {
				KeyFrame k;
				//Add keyframes if world object.
				if (Shared::selectedWorldObject) {
					k.frame = 0;
					k.position = Shared::selectedWorldObject->position;
					k.scale = Shared::selectedWorldObject->scale;
					k.rotationAxis = Shared::selectedWorldObject->rotationAxis;
					k.rotationAngle = Shared::selectedWorldObject->rotationAngle;
				}
				//Add keyframes if npc.
				else if (Shared::selectedNpc) {
					k.frame = 0;
					k.position = Shared::selectedNpc->position;
					k.scale = Shared::selectedNpc->scale;
					k.rotationAxis = Shared::selectedNpc->rotationAxis;
					k.rotationAngle = Shared::selectedNpc->rotationAngle;
				}

				newTrack->keyframes.push_back(k);
			}
		}

		ImGui::NewLine();
		//Save Track to cutscene
		if ((Shared::selectedWorldObject || Shared::selectedNpc) && newTrack) {
			if (ImGui::Button("Save Track") )
			{
				//Save track to world object cutscene
				if (Shared::selectedWorldObject) {
					Shared::cutscenes.worldObjectCutscene[Shared::selectedWorldObject->id].push_back(*newTrack);
				}
				//Save track to npc cutscene
				if (Shared::selectedNpc) {
					Shared::cutscenes.npcCutscene[Shared::selectedNpc->name].push_back(*newTrack);
				}
				newTrack = nullptr;

				Shared::cutscenes.SaveOriginalKeyFrame();
				Shared::cutscenes.Stop();
			}
		}
		ImGui::NewLine();

		//Display Tracks for the selected world object
		for (auto& [id, tracks] : Shared::cutscenes.worldObjectCutscene) {
			ImGui::SeparatorText(("World Object Name: " + Shared::worldObjects[id].modelName).c_str());
			for (int i = 0; i < tracks.size(); i++) {
				auto& track = tracks[i];

				//Speed of the animation Track
				ImGui::InputFloat(("Seconds?##" + id + std::to_string(i)).c_str(), &track.animDuration);
				ImGui::NewLine();

				//Play Track
				if (ImGui::Button(("Play Track: " + std::to_string(i) + " " + track.name).c_str())) {
					Shared::cutscenes.Reset(track.name);
					Shared::cutscenes.playingTrack = track.name;
					Shared::cutscenes.SaveOriginalKeyFrame();

					Shared::isCutscenePlaying = true;
				}
				ImGui::SameLine();

				//Delete Track
				if (ImGui::Button(("Delete Track: " + std::to_string(i) + " " + track.name).c_str())) {
					if (Shared::cutscenes.playingTrack == track.name) {
						Shared::cutscenes.playingTrack = nullptr;
					}
					Shared::cutscenes.worldObjectCutscene[track.worldObjectID].erase(Shared::cutscenes.worldObjectCutscene[track.worldObjectID].begin() + i);

					Shared::cutscenes.Stop();
				}
			}
		}
		

		//Display Tracks for the selected NPC
		for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
			ImGui::SeparatorText(("Npc Name: " + name).c_str());
			for (int i = 0; i < tracks.size(); i++) {
				auto& track = tracks[i];

				//Speed of the animation Track
				ImGui::InputFloat(("Seconds?##" + name + std::to_string(i)).c_str(), &track.animDuration);
				ImGui::NewLine();

				//Play Track
				if (ImGui::Button(("Play Track: " + std::to_string(i) + " " + track.name).c_str())) {
					Shared::cutscenes.Reset(track.name);
					Shared::cutscenes.playingTrack = track.name;
					Shared::cutscenes.SaveOriginalKeyFrame();

					Shared::isCutscenePlaying = true;
				}
				ImGui::SameLine();
				//Delete Track
				if (ImGui::Button(("Delete Track: " + std::to_string(i) + " " + track.name).c_str())) {
					if (Shared::cutscenes.playingTrack == track.name) {
						Shared::cutscenes.playingTrack = nullptr;
					}
					Shared::cutscenes.npcCutscene[track.npcName].erase(Shared::cutscenes.npcCutscene[track.npcName].begin() + i);

					Shared::cutscenes.Stop();
				}
			}
		}

		//Stop playing a track
		if (Shared::cutscenes.playingTrack != "") {
			if (ImGui::Button("Stop playing track")) {

				//Stop the cutscene and reset the entity/npc to its original keyframe. Then Update the collider
				Shared::cutscenes.Stop();

				Shared::cutscenes.playingTrack = "";
				Shared::isCutscenePlaying = false;
			}
		}


		//Play a track
		if (Shared::cutscenes.playingTrack != "") {

			Shared::cutscenes.Play(Shared::cutscenes.playingTrack);
			for (auto& [id, tracks] : Shared::cutscenes.worldObjectCutscene) {
				for (int i = 0; i < tracks.size(); i++) {
					auto& track = tracks[i];
					auto& worldObject = Shared::worldObjects[track.worldObjectID];
					worldObject.collider = Shared::GetUpdatedBoundingBox(worldObject.originalCollider, worldObject.position, worldObject.scale, worldObject.rotationAxis, worldObject.rotationAngle);
				}
			}
			for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
				for (int i = 0; i < tracks.size(); i++) {
					auto& track = tracks[i];
					auto& npc = Shared::npcs[track.npcName];
					npc.collider = Shared::GetUpdatedBoundingBox(npc.originalCollider, npc.position, npc.scale, npc.rotationAxis, npc.rotationAngle);
				}
			}
		}


		//---LIGHTS----
		ImGui::SeparatorText("LIGHTS");

		static int selectedLightType = 0;//Point Light by default
		static const char* lightTypeNames[] = { "Point", "Spot", "Directional" };

		if (ImGui::BeginCombo("Select Light Type", lightTypeNames[selectedLightType])) {

			if (ImGui::Button("Point"))
				selectedLightType = 0;
			if (ImGui::Button("Spot"))
				selectedLightType = 1;
			if (ImGui::Button("Directional"))
				selectedLightType = 2;

			ImGui::EndCombo();
		}
		
		if (ImGui::Button("Add Light"))
			LightSystem::Create(Shared::free.cam.position, (LIGHT_TYPE)selectedLightType);

		ImGui::NewLine();

		//Edit the details of the selected light
		if (LightSystem::selectedLight != nullptr) {

			
			ImGui::Checkbox("Enabled", &LightSystem::selectedLight->enabled);
			ImGui::NewLine();


			if (LightSystem::selectedLight->type == DIRECTIONAL_LIGHT) {
				ImGui::Text("Sun Position");
				ImGui::NewLine();

				ImGui::InputFloat("Pos_X", &LightSystem::selectedLight->position.x);
				ImGui::SameLine();
				ImGui::InputFloat("Pos_Y", &LightSystem::selectedLight->position.y);
				ImGui::SameLine();
				ImGui::InputFloat("Pos_Z", &LightSystem::selectedLight->position.z);
			}

			ImGui::NewLine();
			ImGui::InputFloat("Intensity", &LightSystem::selectedLight->intensity);

			ImGui::NewLine();
			ImGui::InputFloat("Range", &LightSystem::selectedLight->range);

			ImGui::NewLine();
			float col[4] = {
				LightSystem::selectedLight->color.r / 255.0f,
				LightSystem::selectedLight->color.g / 255.0f,
				LightSystem::selectedLight->color.b / 255.0f,
				LightSystem::selectedLight->color.a / 255.0f
			};

			ImGui::NewLine();
			if (ImGui::ColorEdit4("Color", col)) {
				LightSystem::selectedLight->color.r = (unsigned char)(col[0] * 255.0f);
				LightSystem::selectedLight->color.g = (unsigned char)(col[1] * 255.0f);
				LightSystem::selectedLight->color.b = (unsigned char)(col[2] * 255.0f);
				LightSystem::selectedLight->color.a = (unsigned char)(col[3] * 255.0f);
			}

			if (LightSystem::selectedLight->type == SPOT_LIGHT) {
				ImGui::NewLine();
				ImGui::InputFloat("Spot Angle", &LightSystem::selectedLight->spotAngle);
			}

			
		}

		
		//Decal Settings
		ImGui::SeparatorText("DECALS");

		//Dialouges
		ImGui::SeparatorText("DIALOGUES");

		static DialougeText selectedDialougeText;
		static std::string selectedDialougeName = "";
		static std::string selectedDialougeNpc = "";
		ImGui::InputTextMultiline("Dialouge Text", &selectedDialougeText.text);
		ImGui::InputText("Dialouge Name", &selectedDialougeName);
		ImGui::InputText("Dialouge NPC", &selectedDialougeNpc);

		if(ImGui::Button("Add Dialouge"))
		{
			DialougeSystem::AddDialouge(selectedDialougeNpc, selectedDialougeName, selectedDialougeText );
			std::cout<< "Dialouge Text: " << selectedDialougeText.text << std::endl;
			selectedDialougeText.text.clear();
			selectedDialougeName.clear();
			selectedDialougeNpc.clear();
		}

		ImGui::End();

	}

	first = false;
}

void ImguiWindow::UpdateImguiExitPopup(bool& running)
{

	if (IsKeyPressed(KEY_ESCAPE)) {
		ImGui::OpenPopup("Exit?");
		Shared::popupOpen = true;
	}

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Do you want to save before exiting?");

		if (ImGui::Button("Save & Exit"))
		{
			SaveSystem::SaveGame("savegame.json");
			running = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Exit"))
			running = false;

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			Shared::popupOpen = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

	}
}

void ImguiWindow::TrackNamePopup(std::string& name)
{
	ImGui::SetNextWindowPos(
		ImVec2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f)
	);

	if (ImGui::BeginPopupModal("TrackName?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Enter the Track Name: ");

		ImGui::InputText("##TrackName", &Shared::trackName);

		if (ImGui::Button("OK") && !Shared::trackName.empty())
		{
			name = Shared::trackName;
			
			std::cout << "New Track Name: " << name << "\n";
			Shared::popupOpen = false;
			Shared::trackName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

	}
}
