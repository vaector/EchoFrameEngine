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


    //Loop through walls and if hit then add a Decal on the hit position
    float closestWallDistance = INFINITY;
    Vector3 hitWallPos{};
    Wall* hitWall = nullptr;

    for (auto& wall : Shared::walls)
    {
        Vector3 bottomLeft = Shared::ToWorld({ wall->startPos.x, (float)(wall->floor * Shared::globalWallHeight), wall->startPos.y });
        Vector3 bottomRight = Shared::ToWorld({ wall->endPos.x, (float)(wall->floor * Shared::globalWallHeight), wall->endPos.y });
        Vector3 topLeft = Shared::ToWorld({ wall->startPos.x, (float)((wall->floor * Shared::globalWallHeight) + Shared::globalWallHeight), wall->startPos.y });
        Vector3 topRight = Shared::ToWorld({ wall->endPos.x, (float)((wall->floor * Shared::globalWallHeight) + Shared::globalWallHeight), wall->endPos.y });

        RayCollision col = GetRayCollisionQuad(Shared::playerRay, bottomLeft, topLeft, topRight, bottomRight);

        if (col.hit && col.distance < closestWallDistance) { closestWallDistance = col.distance; hitWallPos = col.point; hitWall = wall; }
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
        hitDecal.pos = Vector3Add(hitWallPos, Vector3Scale(normal, 0.75f));
        hitDecal.wallDir = wallDir;
        hitDecal.textureName = "nailHole";
        hitDecal.floor = hitWall->floor;
        hitDecal.scale = 0.1f;

        Shared::decals.push_back(hitDecal);
    }

    //Loop through Sectors(Floors/Ceilings) and if hit then add a Decal on the hit position

    float closestSectorDistance = FLT_MAX;
    Vector3 hitSectorPos{};
    Sector* hitSector = nullptr;
    bool hitCeiling = false;

    for (auto& sector : Shared::sectors)
    {
        float floorY = (float)(sector.floor * Shared::globalWallHeight);
        float ceilingY = floorY + Shared::globalWallHeight - 1.0f;

        // floor
        for (auto j = 0; j + 2 < sector.indices.size(); j += 3)
        {
            int i0 = sector.indices[j];
            int i1 = sector.indices[j + 1];
            int i2 = sector.indices[j + 2];

            Vector3 p0 = Shared::ToWorld({ sector.floorPoints[i0].x, floorY, sector.floorPoints[i0].y });
            Vector3 p1 = Shared::ToWorld({ sector.floorPoints[i1].x, floorY, sector.floorPoints[i1].y });
            Vector3 p2 = Shared::ToWorld({ sector.floorPoints[i2].x, floorY, sector.floorPoints[i2].y });

            RayCollision col = GetRayCollisionTriangle(Shared::playerRay, p0, p1, p2);

            if (col.hit && col.distance < closestSectorDistance)
            {
                closestSectorDistance = col.distance;
                hitSectorPos = col.point;
                hitSector = &sector;
                hitCeiling = false;
            }
        }

        // ceiling
        for (auto j = 0; j + 2 < sector.indices.size(); j += 3)
        {
            int i0 = sector.indices[j];
            int i1 = sector.indices[j + 1];
            int i2 = sector.indices[j + 2];

            Vector3 p0 = Shared::ToWorld({ sector.ceilingPoints[i0].x, ceilingY, sector.ceilingPoints[i0].y });
            Vector3 p1 = Shared::ToWorld({ sector.ceilingPoints[i1].x, ceilingY, sector.ceilingPoints[i1].y });
            Vector3 p2 = Shared::ToWorld({ sector.ceilingPoints[i2].x, ceilingY, sector.ceilingPoints[i2].y });

            RayCollision col = GetRayCollisionTriangle(Shared::playerRay, p0, p1, p2);

            if (col.hit && col.distance < closestSectorDistance)
            {
                closestSectorDistance = col.distance;
                hitSectorPos = col.point;
                hitSector = &sector;
                hitCeiling = true;
            }
        }
    }

    // Create sector decal
    if (hitSector)
    {
        Decal hitDecal;

        Vector3 normal = hitCeiling ? Vector3{ 0, 1, 0 } : Vector3{ 0, -1, 0 };

        hitDecal.pos = Vector3Add(hitSectorPos, Vector3Scale(normal, 0.02f));
        hitDecal.textureName = "nailHole";
        hitDecal.floor = hitSector->floor;
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
