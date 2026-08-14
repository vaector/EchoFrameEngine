#pragma once
#include <unordered_map>
#include <string>

class ImguiWindow
{

public:
	void UpdateImguiTileEditor();
	void UpdateImguiExitPopup(bool& running);
	void TrackNamePopup(std::string& name);

	bool first = true;
	std::unordered_map<int, std::string> layerNameMap;
};

