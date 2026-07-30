#include "ImguiWindow.h"
#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"
#include "Shared.h"
#include "imgui/imgui_stdlib.h"
#include "Entity.h"
#include <iostream>

void ImguiWindow::UpdateImguiTileEditor()
{

	//Set initial values of the LayerNameMap
	if (first) {
		layerNameMap[0] = "Walls";
		layerNameMap[1] = "Sectors";
		layerNameMap[2] = "Objects";
	}

	rlImGuiBegin();

	ImGui::SetNextWindowSizeConstraints(
		ImVec2(200, 200),
		ImVec2(600, 800)
	);

	//------TileMap Editor Window------
	ImGui::Begin("TileMap Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetWindowPos({ 0,0 });

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
			Shared::selectedTexture = &tex;
			selectedT = key;
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

		ImGui::End();
	}
	rlImGuiEnd();

	first = false;
}
