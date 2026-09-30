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
	static int index;//Current index(Line) of the dialouge
	static bool isFinished;//Is the dialouge finished playing?
	static bool isPlaying;//Is the dialouge playing?
	static std::string selectedDialougeNpcName;
	static std::string selectedDialougeName;

	static void DrawDialouge();
	static  void AddDialouge( std::string& npcName,  std::string& dialougeName,  DialougeText& dialougeText);
	static void RenderDialougeTexts( std::string& npcName,  std::string& dialougeName);
	static void StartDialouge(const std::string& npcName, const std::string& dialougeName);
};

