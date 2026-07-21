#include "ImguiWindow.h"
#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"
#include "Shared.h"
#include "imgui/imgui_stdlib.h"
#include "Entity.h"

void ImguiWindow::UpdateImguiTileEditor()
{

	//Set initial values of the LayerNameMap
	if (first) {
		layerNameMap[0] = "Walls";
		layerNameMap[1] = "Sectors";
		layerNameMap[2] = "Objects";
	}

	rlImGuiBegin();

	//------TileMap Editor Window------
	ImGui::Begin("TileMap Editor");

	ImGui::SetWindowSize({ 200,600 }, ImGuiCond_Once);
	ImGui::SetWindowPos({ 0,0 }, ImGuiCond_Once);

	//Drop Down box for choosing textures
	static std::string selectedT = "tile1";
	if (ImGui::BeginCombo("Textures", selectedT.c_str())) {

		for (auto& [key, val] : Shared::textures) {

			bool selected = (key == selectedT);

			if (ImGui::Selectable(key.c_str())) {
				Shared::selectedTexture = &val;
				selectedT = key.c_str();
			}

			if (selected)
				ImGui::SetItemDefaultFocus();

		}
		ImGui::EndCombo();
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

	ImGui::End();

	//------3D Models Window------
	ImGui::Begin("3D Models");
	ImGui::SetWindowSize({ 200,600 }, ImGuiCond_Once);
	ImGui::SetWindowPos({ 880,0 }, ImGuiCond_Once);

	static std::string search;
	static std::string selectedModel;

	ImGui::InputText("Search", &search);

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
				Shared::selectedModel = &selectedModel;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndListBox();
	}
	
	ImGui::End();

	rlImGuiEnd();

	first = false;
}
