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
#include "BoxTrigger.h"

void ImguiWindow::UpdateImguiEditor()
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
	SavePopup(fileName);
	

	// Load
	if (ImGui::Button("Load"))
		ImGui::OpenPopup("Load?");
	LoadPopup(fileName);
	
	ImGui::SeparatorText("Gloabal Wall Height");
	ImGui::InputInt("Wall Height", &Shared::globalWallHeight);
	ImGui::SeparatorText("PROPERTIES");


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

	ImGui::SeparatorText("TEXTURES");

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

	

	ImGui::End();


	ImGui::SetNextWindowSizeConstraints(
		ImVec2(200, 400),
		ImVec2(600, 800)
	);

	if (!Shared::editorMode) {

		//------3D Models Window------
		ImGui::Begin("3D Editor", nullptr, ImGuiChildFlags_AlwaysAutoResize);
		ImGui::SetWindowPos({ (float)GetScreenWidth() - 600,0 });

		//---ON CREATE ENTITY PROPERTIES

		ImGui::SeparatorText("On Create Entity Properties");

		ImGui::InputFloat("X##", &Shared::entityScale.x);
		ImGui::InputFloat("Y##", &Shared::entityScale.y);
		ImGui::InputFloat("Z##", &Shared::entityScale.z);
		
		ImGui::NewLine();

		//--PREOPERTIES SECTION
		ImGui::SeparatorText("WorldObject/NPC Properties");
		//World Object Properties
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

		//Npc Properties
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

		ImGui::SeparatorText("BRUSH MODE");
		ImGui::Checkbox("Brush Mode", &Shared::brushMode);
		if (Shared::brushMode) {
			ImGui::SliderFloat("Brush Radius", &Shared::brushRadius, 10.0f, 500.0f);
			ImGui::SliderInt("Brush Amount", &Shared::brushAmount, 1, 100);
		}

		//Collision Type
		static bool collisionTypePopupOpen = false;
		if(collisionTypePopupOpen)
			ImGui::OpenPopup("CollisionType?");
		CollisionTypePopup(collisionTypePopupOpen);

		//Rotation Axis
		ImGui::SeparatorText("Rotation Axis");

		static bool isRotationAxisX = false;
		static bool isRotationAxisY = true;
		static bool isRotationAxisZ = false;

		ImGui::Checkbox("X", &isRotationAxisX);
		ImGui::SameLine();
		ImGui::Checkbox("Y", &isRotationAxisY);
		ImGui::SameLine();
		ImGui::Checkbox("Z", &isRotationAxisZ);

		Shared::rotationAxis = { (float)isRotationAxisX, (float)isRotationAxisY, (float)isRotationAxisZ };

		ImGui::NewLine();
		ImGui::Checkbox("Is Mesh Instance", &Shared::isMeshInstance);

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
					if (Shared::selectedModelName != nullptr && *Shared::selectedModelName == name)
						Shared::selectedModelName = nullptr;
					else
					{
						selectedModel = name;
						Shared::selectedModelName = &selectedModel;
						collisionTypePopupOpen = true;
					}
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndListBox();
		}
		ImGui::NewLine();


		//NPC mode
		ImGui::Checkbox("NPC Mode", &Shared::npcMode);


		ImGui::NewLine();

		//Animations of selested model
		if (Shared::selectedModelName != nullptr) {
			ImGui::BeginChild("Animations", ImVec2(0, 300));

			for (auto& [name, animNames] : Shared::modelAnimationNames) {
				if (name == *Shared::selectedModelName) {
					for (auto& name: animNames) {
						ImGui::Text(name.c_str());
						ImGui::NewLine();
					}
				}
			}
			ImGui::EndChild();

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

					//Update the collider
					for (int i = 0; i < worldObject.collisionBoxes.size(); i++)
						worldObject.collisionBoxes[i] = Shared::GetUpdatedBoundingBox(worldObject.originalBoxes[i], worldObject.position, worldObject.scale, worldObject.rotationAxis, worldObject.rotationAngle);
				}
			}
			for (auto& [name, tracks] : Shared::cutscenes.npcCutscene) {
				for (int i = 0; i < tracks.size(); i++) {
					auto& track = tracks[i];
					auto& npc = Shared::npcs[track.npcName];

					//Update the collider
					npc.collisionBoxes[0] = Shared::GetUpdatedBoundingBox(npc.originalBoxes[0], npc.position, npc.scale, npc.rotationAxis, npc.rotationAngle);
				}
			}
		}


		//---LIGHTS----
		ImGui::SeparatorText("LIGHTS");

		const float minPos = 0.0f;
		const float maxPos = 6000.0f;
		const  float minIntensity = 0.0f;
		const float maxIntensity = 500.0f;
		const float minRange = 0.0f;
		const float maxRange = 3000.0f;

		static int selectedLightType = 0;//Point Light by default
		static const char* lightTypeNames[] = { "Point", "Spot", "Directional" };

		ImGui::SeparatorText("Default Light");
		ImGui::Checkbox("Default Directional Light", &LightSystem::defaultLight.enabled);

		ImGui::PushID("DefaultLight");
		if (LightSystem::defaultLight.enabled) {

			ImGui::Text("Default Light Properties");
			ImGui::NewLine();

			ImGui::Text("Sun Position");

			ImGui::SliderFloat("Pos_X", &LightSystem::defaultLight.position.x, minPos, maxPos);
			ImGui::SliderFloat("Pos_Y", &LightSystem::defaultLight.position.y, minPos, maxPos);
			ImGui::SliderFloat("Pos_Z", &LightSystem::defaultLight.position.z, minPos, maxPos);

			ImGui::NewLine();
			ImGui::SliderFloat("Intensity", &LightSystem::defaultLight.intensity, minIntensity, maxIntensity);

			ImGui::NewLine();
			ImGui::SliderFloat("Range", &LightSystem::defaultLight.range, minRange, maxRange);

			ImGui::NewLine();
			float col[4] = {
				LightSystem::defaultLight.color.r / 255.0f,
				LightSystem::defaultLight.color.g / 255.0f,
				LightSystem::defaultLight.color.b / 255.0f,
				LightSystem::defaultLight.color.a / 255.0f
			};

			ImGui::NewLine();
			if (ImGui::ColorEdit4("Color", col)) {
				LightSystem::defaultLight.color.r = (unsigned char)(col[0] * 255.0f);
				LightSystem::defaultLight.color.g = (unsigned char)(col[1] * 255.0f);
				LightSystem::defaultLight.color.b = (unsigned char)(col[2] * 255.0f);
				LightSystem::defaultLight.color.a = (unsigned char)(col[3] * 255.0f);
			}



		}
		ImGui::PopID();

		ImGui::NewLine();

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

		ImGui::SeparatorText("SELECTED LIGHT");
		ImGui::PushID("SelectedLight");
		//Edit the details of the selected light
		if (LightSystem::selectedLight != nullptr) {

			
			ImGui::Checkbox("Enabled", &LightSystem::selectedLight->enabled);
			ImGui::NewLine();


			if (LightSystem::selectedLight->type == DIRECTIONAL_LIGHT) {
				ImGui::Text("Sun Position");

				ImGui::SliderFloat("Pos_X", &LightSystem::selectedLight->position.x, minPos, maxPos);
				ImGui::SliderFloat("Pos_Y", &LightSystem::selectedLight->position.y, minPos, maxPos);
				ImGui::SliderFloat("Pos_Z", &LightSystem::selectedLight->position.z, minPos, maxPos);
			}

			ImGui::NewLine();
			ImGui::SliderFloat("Intensity", &LightSystem::selectedLight->intensity, minIntensity, maxIntensity);

			ImGui::NewLine();
			ImGui::SliderFloat("Range", &LightSystem::selectedLight->range, minRange, maxRange);

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
		ImGui::PopID();
		
		//Decal Settings
		ImGui::SeparatorText("DECALS");
		if (Shared::selectedDecal) {
			ImGui::Text("Selected Decal Properties");
			ImGui::NewLine();
			ImGui::Text(("Texture Name: " + Shared::selectedDecal->textureName).c_str());
			ImGui::NewLine();
			ImGui::Text("Position");
			ImGui::InputFloat3("##Position", &Shared::selectedDecal->pos.x);
			ImGui::NewLine();
			ImGui::Text("Scale");
			ImGui::InputFloat("##Scale", &Shared::selectedDecal->scale);
		}

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
		}

		//Trigger Settings
		ImGui::SeparatorText("TRIGGERS");

		static std::string triggerName = "";
		ImGui::InputText("Trigger Name", &triggerName);

		if(ImGui::Button("Add Trigger"))
		{
			BoxTrigger trigger;
			//Position
			trigger.pos = Shared::free.cam.position;
			
			//Bounding Box
			trigger.box.min = Vector3Subtract(trigger.pos, Vector3Scale(trigger.size, 0.5f));
			trigger.box.max = Vector3Add(trigger.pos, Vector3Scale(trigger.size, 0.5f));

			//Name
			trigger.name = triggerName;

			Shared::boxTriggers[trigger.name] = trigger;
		}

		//Selected Trigger Details
		if (Shared::selectedBoxTrigger) {
			ImGui::SeparatorText("Trigger Details");

			//Size
			ImGui::Text("Size");
			ImGui::SliderFloat("X##TriggerSizeX", &Shared::selectedBoxTrigger->size.x, 10, 1000);
			ImGui::SliderFloat("Y##TriggerSizeY", &Shared::selectedBoxTrigger->size.y, 10, 1000);
			ImGui::SliderFloat("Z##TriggerSizeZ", &Shared::selectedBoxTrigger->size.z, 10, 1000);

			//X axis of the Size
			if (IsKeyDown(KEY_X)) {
				if (IsKeyPressed(KEY_RIGHT))
					Shared::selectedBoxTrigger->size.x += 10;
				else if (IsKeyPressed(KEY_LEFT))
					Shared::selectedBoxTrigger->size.x -= 10;
			}
			//Y axis of the Size
			else if (IsKeyDown(KEY_Y)) {
				if (IsKeyPressed(KEY_RIGHT))
					Shared::selectedBoxTrigger->size.y += 10;
				else if (IsKeyPressed(KEY_LEFT))
					Shared::selectedBoxTrigger->size.y -= 10;
			}
			//Z axis of the Size
			else if (IsKeyDown(KEY_Z)) {
				if (IsKeyPressed(KEY_RIGHT))
					Shared::selectedBoxTrigger->size.z += 10;
				else if (IsKeyPressed(KEY_LEFT))
					Shared::selectedBoxTrigger->size.z -= 10;
			}

			Shared::selectedBoxTrigger->size = Vector3Clamp(Shared::selectedBoxTrigger->size, { 10,10,10 }, { 1000,1000,1000 });
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

	ImGui::SetNextWindowPos(
		ImVec2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f)
	);

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

void ImguiWindow::LoadPopup(std::string& fileName)
{
	ImGui::SetNextWindowPos(
		ImVec2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f)
	);

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
}

void ImguiWindow::SavePopup(std::string& fileName)
{
	ImGui::SetNextWindowPos(
		ImVec2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f)
	);

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

void ImguiWindow::CollisionTypePopup(bool& collisionTypePopupOpen)
{

	ImGui::SetNextWindowPos(
		ImVec2(GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f),
		ImGuiCond_Appearing,
		ImVec2(0.5f, 0.5f)
	);

	if (ImGui::BeginPopupModal("CollisionType?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Select Collision Type:");

		
		if (ImGui::Button("NONE")) {
			Shared::selectedCollisionType = CollisionType::NONE;
			collisionTypePopupOpen = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("BOX")) {
			Shared::selectedCollisionType = CollisionType::BOX;
			collisionTypePopupOpen = false;
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::Button("MESH")) {
			Shared::selectedCollisionType = CollisionType::MESH;
			collisionTypePopupOpen = false;
			ImGui::CloseCurrentPopup();
		}
		

		ImGui::EndPopup();

	}
}
