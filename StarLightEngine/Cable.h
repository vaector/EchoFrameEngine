#pragma once
#include <string>

//Cable Gun
enum CableState
{
	FirstEndpointAttached,
	Connected
};
class Cable
{

public:
	std::string startSocket = "";
	std::string endSocket = "";
	CableState state = FirstEndpointAttached;

	static bool Connected(std::string firstSocket, std::string secondSocket);
	static void OpenDoor(int doorId);
};

