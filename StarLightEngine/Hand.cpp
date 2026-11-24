#include "Hand.h"
#include "Audio.h"

void Hand::CreateHand(std::string name)
{
	textureName = name;

}

void Hand::UpdateHand()
{
    UpdateNailShooting();

}

void Hand::UpdateNailShooting()
{
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

    float closestDistance = INFINITY;
    Vector3 hitPos{};
    Wall* hitWall = nullptr;

    for (auto& wall : Shared::walls)
    {
        Vector3 bottomLeft = Shared::ToWorld({ wall->startPos.x, (float)(wall->floor * Shared::globalWallHeight), wall->startPos.y });
        Vector3 bottomRight = Shared::ToWorld({ wall->endPos.x, (float)(wall->floor * Shared::globalWallHeight), wall->endPos.y });
        Vector3 topLeft = Shared::ToWorld({ wall->startPos.x, (float)((wall->floor * Shared::globalWallHeight) + Shared::globalWallHeight), wall->startPos.y });
        Vector3 topRight = Shared::ToWorld({ wall->endPos.x, (float)((wall->floor * Shared::globalWallHeight) + Shared::globalWallHeight), wall->endPos.y });

        RayCollision col = GetRayCollisionQuad(Shared::playerRay, bottomLeft, topLeft, topRight, bottomRight);

        if (col.hit && col.distance < closestDistance) { closestDistance = col.distance; hitPos = col.point; hitWall = wall; }
    }

    if (hitWall)
    {
        Vector3 start = Shared::ToWorld({ hitWall->startPos.x, 0, hitWall->startPos.y });
        Vector3 end = Shared::ToWorld({ hitWall->endPos.x, 0, hitWall->endPos.y });
        Vector3 direction = Vector3Normalize(Vector3Subtract(end, start));
        Vector2 wallDir = { direction.x, direction.z };

        Vector3 normal = Vector3CrossProduct({ wallDir.x, 0, wallDir.y }, { 0, 1, 0 });
        if (Vector3DotProduct(Shared::playerRay.direction, normal) > 0.0f) normal = Vector3Scale(normal, -1.0f);

        Decal hitDecal{};
        hitDecal.pos = Vector3Add(hitPos, Vector3Scale(normal, 0.75f));
        hitDecal.wallDir = wallDir;
        hitDecal.textureName = "nailHole";
        hitDecal.floor = hitWall->floor;
        hitDecal.scale = 0.1f;

        Shared::decals.push_back(hitDecal);
    }

    //PLay shooting sfx
	Audio::Play(Shared::sounds["nailGunShoot"]);
}

void Hand::DrawHand()
{
	//Render Hand
	float width = GetScreenWidth()-(Shared::textures[textureName].width)*1.5;
	float height = GetScreenHeight()-(Shared::textures[textureName].height)*1.5;
	DrawTextureEx(Shared::textures[textureName], {width, height}, -15.f, 1.5, WHITE);
}
