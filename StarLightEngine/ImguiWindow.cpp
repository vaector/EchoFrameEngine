#include "ImguiWindow.h"
#include "raylib.h"
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"
#include "Shared.h"
#include <string>

void ImguiWindow::UpdateImguiTileEditor()
{
	rlImGuiBegin();
	ImGui::Begin("TileMap Editor");

	ImGui::SetWindowSize({ 200,600 });
	ImGui::SetWindowPos({ 0,0 });

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
	ImGui::Checkbox("Sector Mode", &Shared::sectorMode);
	ImGui::Checkbox("Floor Only", &Shared::floorOnly);
	ImGui::Checkbox("Ceiling Only", &Shared::ceilingOnly);
	ImGui::NewLine();

	//Current Floor
	if (ImGui::InputInt("Floor", &Shared::floor)) {
		if (Shared::floor <= 0)
			Shared::floor = 0;
	}

	//Door Mode
	ImGui::Checkbox("Door Mode", &Shared::doorMode);

	ImGui::End();
	rlImGuiEnd();
}
