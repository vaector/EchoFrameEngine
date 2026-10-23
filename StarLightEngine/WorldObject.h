#pragma once
#include "Entity.h"
#include <unordered_map>

/*
-------------------------
	For Mesh Instancing, we will store the transforms of each instance in a batch.
	When the batch is dirty, we will update the instance buffer with the new transforms. 
	This way, we can render all instances in a single draw call, improving performance.
---------------------------
*/
struct InstanceBatch
{
	std::vector<Matrix> transforms;
	bool dirty = true;
};

class WorldObject : public Entity
{

public:
	int id = 0;
	bool isMeshInstance = false;

	static int currentId;
	static std::unordered_map<std::string, InstanceBatch> batches;

};

