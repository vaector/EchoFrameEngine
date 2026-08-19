#pragma once
#include "Entity.h"
class WorldObject : public Entity
{

public:
	int id = 0;
	static int currentId;
};

