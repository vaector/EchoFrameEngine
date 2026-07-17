#pragma once
#include <unordered_map>
#include <string>

class ImguiWindow
{

public:
	void UpdateImguiTileEditor();

	bool first = true;
	std::unordered_map<int, std::string> layerNameMap;
};

