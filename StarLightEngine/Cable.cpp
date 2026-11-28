#include "Cable.h"
#include "GameShared.h"

bool Cable::Connected(std::string firstSocket, std::string secondSocket)
{
	for(const auto& cable : GameShared::cables)
	{
		if((cable.startSocket == firstSocket && cable.endSocket == secondSocket) ||
		   (cable.startSocket == secondSocket && cable.endSocket == firstSocket))
		{
			return true;
		}
	}

	return false;
}

void Cable::OpenDoor(int doorId)
{
	for (auto& wall : Shared::walls) {
		if (wall->id != doorId) continue;

		//Found the wall check if it is open and if it is NOT a door. if true then return.
		if (!wall->isDoor) return;//Check if it is a door
		if (wall->isOpen) return;//check if it is already open

		//If the wall is a door and it is closed then open it.
		wall->isOpen = true;

	}
}