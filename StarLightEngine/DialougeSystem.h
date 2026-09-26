#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

struct DialougeText {
	std::string text = "";
	Color textColor = WHITE;
	int fontSize = 20;
	Vector2 position = { 0, 0 };	
};

class DialougeSystem
{
public:
	static std::unordered_map<std::string, std::unordered_map<std::string, std::vector<DialougeText>>> dialouges;//unordered_map<NPC Name, unordered_map<Dialouge Name, vector<DialougeText>>>

	static void DrawDialouge();
	static  void AddDialouge(const std::string& npcName, const std::string& dialougeName, const DialougeText& dialougeText);
	static void RenderDialouge(const std::string& npcName, const std::string& dialougeName, int index);
};

