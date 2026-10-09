#pragma once
#include "Trigger.h"

class BoxTrigger : public Trigger
{
public:
	Vector3 size = { 100, 100, 100 };//Size of the box trigger in 3D space
	BoundingBox box;

	bool IsInside(const Vector3& point);//Check if a point is inside the box trigger
	bool IsRayHit(const Ray& ray);//Check if a ray intersects with the box trigger
	void UpdateBox();//Update the bounding box based on the position and size of the box trigger
};

