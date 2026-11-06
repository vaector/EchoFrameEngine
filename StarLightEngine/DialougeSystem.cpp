#pragma once
#include "DialougeSystem.h"
#include "Shared.h"
#include <iostream>


std::unordered_map<std::string, std::unordered_map<std::string, std::vector<DialougeText>>> DialougeSystem::dialouges;
int DialougeSystem::index = 0;
bool DialougeSystem::isFinished = false;
bool DialougeSystem::isPlaying = false;
std::string DialougeSystem::selectedDialougeNpcName = "";
std::string DialougeSystem::selectedDialougeName = "";

void DialougeSystem::DrawDialouge()
{
	DrawRectangle(GetScreenWidth() / 6, GetScreenHeight()/1.5, GetScreenWidth()/1.5, GetScreenHeight()/4, {50, 50, 50, 200});
}

void DialougeSystem::AddDialouge(std::string& npcName, std::string& dialougeName,DialougeText& dialougeText)
{
	DialougeSystem::dialouges[npcName][dialougeName].push_back(dialougeText);
}

void DialougeSystem::RenderDialougeTexts(std::string& npcName, std::string& dialougeName)
{
	if (!Shared::dialougeMode) return;

	auto& texts = DialougeSystem::dialouges[npcName][dialougeName];

	//Increment the index
	if (IsKeyPressed(KEY_SPACE))
		DialougeSystem::index++;

	//Reset
	if (DialougeSystem::index >= texts.size()) {
		DialougeSystem::index = 0;
		DialougeSystem::isFinished = true;
		DialougeSystem::isPlaying = false;
		DialougeSystem::selectedDialougeName = "";
		DialougeSystem::selectedDialougeNpcName = "";

		Shared::dialougeMode = false;

		return;
	}

	//Draw the text on the dialouge box
	DialougeText& dialougeText = texts[DialougeSystem::index];
	dialougeText.position.x = GetScreenWidth() / 6 + dialougeText.fontSize;
	dialougeText.position.y = GetScreenHeight() / 1.5 + dialougeText.fontSize;
	DrawText(dialougeText.text.c_str(), dialougeText.position.x, dialougeText.position.y, dialougeText.fontSize, dialougeText.textColor);
	
	//std::cout << DialougeSystem::index << std::endl;
}
void DialougeSystem::StartDialouge(const std::string& npcName, const std::string& dialougeName)
{
		DialougeSystem::index = 0;
		DialougeSystem::isPlaying = true;
		DialougeSystem::isFinished = false;
		DialougeSystem::selectedDialougeNpcName = npcName;
		DialougeSystem::selectedDialougeName = dialougeName;

		Shared::dialougeMode = true;

}