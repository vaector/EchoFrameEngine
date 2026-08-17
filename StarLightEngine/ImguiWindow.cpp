#include "ImguiWindow.h"
#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"
#include "Shared.h"
#include "imgui/imgui_stdlib.h"
#include "Entity.h"
#include <iostream>
#include "SaveSystem.h"

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
	// Save
	if (ImGui::Button("Save"))
		ImGui::OpenPopup("Save?");

	if (ImGui::BeginPopupModal("Save?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to overwrite the save?");

		if (ImGui::Button("Yes"))
		{
			SaveSystem::SaveGame("savegame.json");
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
			SaveSystem::LoadGame("savegame.json");
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
		ImGui::SeparatorText("Entity/NPC Properties");
		//Entity
		if (Shared::selectedEntity) {
			ImGui::Text("Entity Properties");
			ImGui::NewLine();
			ImGui::Text(("ID: " + std::to_string(Shared::selectedEntity->id)).c_str());
			ImGui::Text(("Model Name: " + Shared::selectedEntity->modelName).c_str());
			ImGui::NewLine();
			ImGui::Text("Position");
			ImGui::InputFloat3("##Position", &Shared::selectedEntity->position.x);
			ImGui::NewLine();
			ImGui::Text("Scale");
			ImGui::InputFloat3("##Scale", &Shared::selectedEntity->scale.x);
			ImGui::NewLine();
			ImGui::Text("Rotation");
			ImGui::InputFloat3("##Rotation", &Shared::selectedEntity->rotation.x);
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
			ImGui::InputFloat3("##Rotation", &Shared::selectedNpc->rotation.x);
		}
		
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

		//--------CUTSCENES FOR ENTITIES-----------
		ImGui::SeparatorText("Cutscenes Section");

		//Add Track
		static Track* newTrack = nullptr;
		
		if (Shared::selectedEntity || Shared::selectedNpc && !newTrack) {
			if (ImGui::Button("Add Track")) {

				newTrack = new Track();
				//Add the entity id to the track
				if (Shared::selectedEntity) {
					newTrack->entityId = Shared::selectedEntity->id;
				}
				//Add the npc name to the track
				if (Shared::selectedNpc) {
					newTrack->npcName = Shared::selectedNpc->name;
				}
				ImGui::OpenPopup("TrackName?");
			}
		}
		if (newTrack != nullptr) {
			TrackNamePopup(newTrack->name);
		}
		ImGui::NewLine();


		//Add Keyframes
		if ((Shared::selectedEntity || Shared::selectedNpc) && newTrack) {
			if (ImGui::Button("Add KeyFrame")) {
				KeyFrame k;
				//Add keyframes if entity.
				if (Shared::selectedEntity) {
					k.frame = 0;
					k.position = Shared::selectedEntity->position;
					k.scale = Shared::selectedEntity->scale;
					k.rotation = Shared::selectedEntity->rotation;
				}
				//Add keyframes if npc.
				else if (Shared::selectedNpc) {
					k.frame = 0;
					k.position = Shared::selectedNpc->position;
					k.scale = Shared::selectedNpc->scale;
					k.rotation = Shared::selectedNpc->rotation;
				}

				newTrack->keyframes.push_back(k);
			}
		}

		ImGui::NewLine();
		//Save Track to cutscene
		if ((Shared::selectedEntity || Shared::selectedNpc) && newTrack) {
			if (ImGui::Button("Save Track") )
			{
				//Save track to entity cutscene
				if (Shared::selectedEntity) {
					Shared::cutscenes.entityCutscene[Shared::selectedEntity->id].push_back(*newTrack);
				}
				//Save track to npc cutscene
				if (Shared::selectedNpc) {
					Shared::cutscenes.npcCutscene[Shared::selectedNpc->name].push_back(*newTrack);
				}
				newTrack = nullptr;
			}
		}
		ImGui::NewLine();

		//Display Tracks for the selected entity
		for (auto& [id, tracks] : Shared::cutscenes.entityCutscene) {
			ImGui::SeparatorText(("Entity Name: " + Shared::entities[id].modelName).c_str());
			for (int i = 0; i < tracks.size(); i++) {
				auto& track = tracks[i];
				auto& entity = Shared::entities[track.entityId];

				//Speed of the animation Track
				ImGui::InputFloat("Seconds?", &track.animSpeed);
				ImGui::NewLine();

				//Play Track
				if (ImGui::Button(("Play Track: " + std::to_string(i) + " " + track.name).c_str())) {
					Shared::cutscenes.playingTrack = &track.name;
					Shared::cutscenes.SaveOriginalKeyFrame();

					Shared::isCutscenePlaying = true;
				}
				ImGui::SameLine();

				//Delete Track
				if (ImGui::Button(("Delete Track: " + std::to_string(i) + " " + track.name).c_str())) {
					if (Shared::cutscenes.playingTrack == &track.name) {
						Shared::cutscenes.playingTrack = nullptr;
					}
					Shared::cutscenes.entityCutscene[track.entityId].erase(Shared::cutscenes.entityCutscene[track.entityId].begin() + i);

					Shared::cutscenes.Stop();
				}
			}
		}
		

		//Display Tracks for the selected NPC
		for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
			ImGui::SeparatorText(("Npc Name: " + name).c_str());
			for (int i = 0; i < tracks.size(); i++) {
				auto& track = tracks[i];
				auto& npc = Shared::npcs[track.npcName];

				//Speed of the animation Track
				ImGui::InputFloat("Seconds?", &track.animSpeed);
				ImGui::NewLine();

				//Play Track
				if (ImGui::Button(("Play Track: " + std::to_string(i) + " " + track.name).c_str())) {
					Shared::cutscenes.playingTrack = &track.name;
					Shared::cutscenes.SaveOriginalKeyFrame();

					Shared::isCutscenePlaying = true;
				}
				ImGui::SameLine();
				//Delete Track
				if (ImGui::Button(("Delete Track: " + std::to_string(i) + " " + track.name).c_str())) {
					if (Shared::cutscenes.playingTrack == &track.name) {
						Shared::cutscenes.playingTrack = nullptr;
					}
					Shared::cutscenes.npcCutscene[track.npcName].erase(Shared::cutscenes.npcCutscene[track.npcName].begin() + i);

					Shared::cutscenes.Stop();
				}
			}
		}

		//Stop playing a track
		if (Shared::cutscenes.playingTrack) {
			if (ImGui::Button("Stop playing track")) {

				//Stop the cutscene and reset the entity/npc to its original keyframe. Then Update the collider
				Shared::cutscenes.Stop();

				Shared::cutscenes.playingTrack = nullptr;
				Shared::isCutscenePlaying = false;
			}
		}


		//Play a track
		if (Shared::cutscenes.playingTrack) {

			Shared::cutscenes.Play(*Shared::cutscenes.playingTrack);
			for (auto& [id, tracks] : Shared::cutscenes.entityCutscene) {
				for (int i = 0; i < tracks.size(); i++) {
					auto& track = tracks[i];
					auto& entity = Shared::entities[track.entityId];
					entity.collider = Shared::GetUpdatedBoundingBox(entity.originalCollider, entity.position, entity.scale);
				}
			}
			for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
				for (int i = 0; i < tracks.size(); i++) {
					auto& track = tracks[i];
					auto& npc = Shared::npcs[track.npcName];
					npc.collider = Shared::GetUpdatedBoundingBox(npc.originalCollider, npc.position, npc.scale);
				}
			}
		}
		
		
		ImGui::End();

	}

	first = false;
}

void ImguiWindow::UpdateImguiExitPopup(bool& running)
{

	if (IsKeyPressed(KEY_ESCAPE)) {
		ImGui::OpenPopup("Exit?");
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

		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

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
			Shared::trackName.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();

	}
}
