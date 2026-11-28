#include "Hand.h"
#include "Audio.h"
#include  "GameShared.h"
#include <iostream>
#include "Cable.h"

std::string Hand::GetHitSocketName()
{
    for (auto& [name, trigger] : Shared::boxTriggers)
    {
        if (name.find("socket") != std::string::npos &&
            !IsSocketUsed(name) &&
            trigger.IsRayHit(Shared::playerRay))
            return name;
    }

    return "";
}

bool Hand::IsSocketUsed(const std::string& socketName)
{
    for (const auto& cable : GameShared::cables)
    {
        if (cable.startSocket == socketName || cable.endSocket == socketName)
            return true;
    }

    return false;
}

void Hand::CreateHand(std::string name)
{
	textureName = name;

}

void Hand::UpdateHand()
{
    UpdateFinger();

    //Nail Gun(Index Finger)
    UpdateNailShooting();

	//Cable Gun(Ring Finger)
    UpdateCableShooting();


}

void Hand::UpdateNailShooting()
{
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || GameShared::hand.currentFinger != IndexFinger) return;

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
        hitDecal.axisH = { wallDir.x, 0, wallDir.y };
        hitDecal.axisV = { 0, 1, 0 };
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
        Decal hitDecal{};

        Vector3 normal = hitCeiling ? Vector3{ 0, -1, 0 } : Vector3{ 0, 1, 0 };

        hitDecal.pos = Vector3Add(hitSectorPos, Vector3Scale(normal, 0.75f));
        hitDecal.axisH = { 1, 0, 0 };
        hitDecal.axisV = hitCeiling ? Vector3{ 0, 0, -1 } : Vector3{ 0, 0, 1 };
        hitDecal.textureName = "nailHole";
        hitDecal.floor = hitSector->floor;
        hitDecal.scale = 0.1f;

        Shared::decals.push_back(hitDecal);
    }

    // Play shooting sfx
    Audio::Play(Shared::sounds["nailGunShoot"]);
}

void Hand::UpdateCableShooting()
{
    if (GameShared::hand.currentFinger != RingFinger) return;


	//Begin cable shooting process
    
    //Attach the First Endpoint
    if (pendingCableIndex == -1) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;
        Cable cable;
        cable.endSocket = GetHitSocketName();
        if (cable.endSocket.empty()) return;
        cable.state = CableState::FirstEndpointAttached;

        GameShared::cables.push_back(cable);
        pendingCableIndex = GameShared::cables.size() - 1; //Last cable added(Current cable)
    }
    //Attach the Second Endpoint
    else {
        if (!IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) return;
        Cable* cable = &GameShared::cables.at(pendingCableIndex);

        //The Second edpoint can't be the First endpoint.
        std::string socket = GetHitSocketName();
        if (socket.empty() || socket == cable->endSocket) return;

        cable->startSocket = socket;

        cable->state = CableState::Connected;
        pendingCableIndex = -1;
    }

}

void Hand::UpdateFinger()
{
    if(IsKeyPressed(KEY_ONE))
        currentFinger = IndexFinger;
    else if(IsKeyPressed(KEY_TWO))
        currentFinger = MiddleFinger;
    else if(IsKeyPressed(KEY_THREE))
        currentFinger = RingFinger;
    else if(IsKeyPressed(KEY_FOUR))
        currentFinger = PinkyFinger;
}

void Hand::DrawHand()
{
    // Render Hand
    float width = GetScreenWidth() - (Shared::textures[textureName].width) * 1.25f;
    float height = GetScreenHeight() - (Shared::textures[textureName].height) * 1.25f;

    static float rotation = -15.f;
	static float posY = height;
    static float animationTime = 1.0f;

    // Start shooting animation
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        animationTime = 0.0f;

    // Update animation
    if (animationTime < 1.0f)
    {
        animationTime += GetFrameTime() / 0.2f;
        animationTime = std::min(animationTime, 1.0f);

        // Move forward, then return
        float t = (animationTime < 0.5f) ? animationTime * 2.0f : (1.0f - animationTime) * 2.0f;

        // Smooth animation
        t = t * t * (3.0f - 2.0f * t);

        rotation = Lerp(-15.f, 30.f, t);
        posY = Lerp(height, height + 50.0f, t);
    }
    else
    {
        rotation = -15.f;
		posY = height;
    }

    DrawTextureEx(Shared::textures[textureName], { width, posY }, rotation, 1.5, WHITE);
}

void Hand::DrawCable()
{
    for (const auto& cable : GameShared::cables)
    {
        auto start = Shared::boxTriggers[cable.startSocket].pos;
        auto end = Shared::boxTriggers[cable.endSocket].pos;

        if (cable.state == CableState::Connected)
            DrawCylinderEx(start, end, 0.05f, 0.05f, 8, RED);
        else if(cable.state == CableState::FirstEndpointAttached)
            DrawCylinderEx(Vector3Subtract(Shared::playerRay.position, {0,2,0}), end, 0.05f, 0.05f, 2, RED);
        
    }
}
