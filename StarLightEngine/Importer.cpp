#include "Importer.h"

#include <filesystem>
#include <iostream>
#include <string>
#include "raylib.h"
#include "Shared.h"
#include <rlgl.h>


namespace fs = std::filesystem;

void Importer::InitTextures()
{
	if (!fs::exists("images"))
		TraceLog(LOG_FATAL, "ERROR::IMAGES_FOLDER_DOES_NOT_EXIST");
	else {
		for (auto& entry : fs::directory_iterator("images")) {
			std::string fileName = entry.path().stem().string(); //stem-> removes the extension.

			Shared::textures[fileName] = LoadTexture(entry.path().string().c_str());

			rlTextureParameters(Shared::textures[fileName].id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
			rlTextureParameters(Shared::textures[fileName].id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
				
			//std::cout << fileName<< std::endl;
		}
	}
}

void Importer::InitModels()
{

	if (!fs::exists("models"))
		TraceLog(LOG_FATAL, "ERROR::MODELS_FOLDER_DOES_NOT_EXIST");
	else {
		for (auto& entry : fs::directory_iterator("models")) {
			std::string fileName = entry.path().stem().string(); //stem-> removes the extension.

			Shared::models[fileName] = LoadModel(entry.path().string().c_str());
			//std::cout << fileName << std::endl;
		}
	}
}

void Importer::InitModelAnimationNames()
{
	if (!fs::exists("models"))
		TraceLog(LOG_FATAL, "ERROR::MODELS_FOLDER_DOES_NOT_EXIST");
	else {
		for (auto& entry : fs::directory_iterator("models")) {
			std::string fileName = entry.path().stem().string(); //stem-> removes the extension.

			int animCount = 0;
			ModelAnimation* anims = LoadModelAnimations(entry.path().string().c_str(), &animCount);

			if (animCount > 0 && anims != nullptr)//there is an animations..
			{
				for (int i = 0; i < animCount; i++) {
					Shared::modelAnimationNames[fileName].push_back(anims[i].name);
					//std::cout << anims[i].name << " " << fileName << std::endl;
				}
			}
		}
	}
}
