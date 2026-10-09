#include "BoxTrigger.h"

bool BoxTrigger::IsInside(const Vector3& point)
{
	return (point.x >= pos.x - size.x / 2 && point.x <= pos.x + size.x / 2) &&
		(point.y >= pos.y - size.y / 2 && point.y <= pos.y + size.y / 2) &&
		(point.z >= pos.z - size.z / 2 && point.z <= pos.z + size.z / 2);
}

bool BoxTrigger::IsRayHit(const Ray& ray)
{

	RayCollision rayCol = GetRayCollisionBox(ray, box);
	if (rayCol.hit)
		return true;

	return false;
}

void BoxTrigger::UpdateBox()
{
	box.min = Vector3Subtract(pos, Vector3Scale(size, 0.5f));
	box.max = Vector3Add(pos, Vector3Scale(size, 0.5f));
}

