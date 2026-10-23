#include "WorldObject.h"
int WorldObject::currentId = 0;
std::unordered_map<std::string, InstanceBatch> WorldObject::batches;