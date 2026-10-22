#pragma once
#include <unordered_map>
#include <string>

class ImguiWindow
{

public:
	void UpdateImguiEditor();
	void UpdateImguiExitPopup(bool& running);
	void LoadPopup(std::string& fileName);
	void SavePopup(std::string& fileName);
	void DeletePopup();
	void TrackNamePopup(std::string& name);
	void CollisionTypePopup(bool& collisionTypePopupOpen);

	bool first = true;
	std::unordered_map<int, std::string> layerNameMap;
};

