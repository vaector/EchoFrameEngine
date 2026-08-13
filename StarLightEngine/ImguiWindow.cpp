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

	//------3D Models Window------
	if (!Shared::editorMode) {
		ImGui::Begin("3D Models", nullptr, ImGuiChildFlags_AlwaysAutoResize);
		ImGui::SetWindowPos({ (float)GetScreenWidth() - 600,0 });

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
		
		if (ImGui::Button("Add Track")) {\
			newTrack = new Track();
		}
		ImGui::NewLine();
		//Add Keyframes
		if (Shared::selectedEntity && newTrack) {

			if (ImGui::Button("Add KeyFrame")) {
				KeyFrame k;
				k.frame = 0;
				k.position = Shared::selectedEntity->position;
				k.scale = Shared::selectedEntity->scale;

				newTrack->keyframes.push_back(k);
				
			}

		}

		ImGui::NewLine();
		//Save Track to cutscene
		if (newTrack && Shared::selectedEntity) {
			if (ImGui::Button("Save Track"))
			{
				Shared::cutscenes.entityCutscene[Shared::selectedEntity->id].push_back(*newTrack);
				newTrack = nullptr;
			}
		}
		
		//Display Tracks for each Entity
		ImGui::NewLine();
		for(auto& [id, tracks] : Shared::cutscenes.entityCutscene) {
			ImGui::Separator();
			ImGui::Text(Shared::entities[id].modelName.c_str());

			for(auto& track : tracks) {
				ImGui::Text("Track with %d keyframes", track.keyframes.size());
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
