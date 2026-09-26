#pragma once
#include "DialougeSystem.h"
#include "Shared.h"


std::unordered_map<std::string, std::unordered_map<std::string, std::vector<DialougeText>>> DialougeSystem::dialouges;

void DialougeSystem::DrawDialouge()
{
	if (!Shared::dialougeMode) return;
	DrawRectangle(GetScreenWidth() / 4, GetScreenHeight()/1.5, GetScreenWidth()/2, GetScreenHeight()/4, {130, 130, 130, 150});
}

void DialougeSystem::AddDialouge(const std::string& npcName, const std::string& dialougeName, const DialougeText& dialougeText)
{
	DialougeSystem::dialouges[npcName][dialougeName].push_back(dialougeText);
}

void DialougeSystem::RenderDialouge(const std::string& npcName, const std::string& dialougeName, int index)
{
	for (int i =0; i < DialougeSystem::dialouges[npcName][dialougeName].size(); i++) {
		if (i != index) continue;
		DialougeText& dialougeText = DialougeSystem::dialouges[npcName][dialougeName][i];

		dialougeText.position.x = GetScreenWidth() / 4 + 20;
		dialougeText.position.y = GetScreenHeight() / 2 + 20 + (i * (dialougeText.fontSize + 10));
		DrawText(dialougeText.text.c_str(), dialougeText.position.x, dialougeText.position.y, dialougeText.fontSize, dialougeText.textColor);
		
	}
}
