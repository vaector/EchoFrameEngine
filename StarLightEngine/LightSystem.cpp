#include "LightSystem.h"
#include "Shared.h"
#include "raymath.h"
#include <algorithm>

Light* LightSystem::selectedLight = nullptr;
Light LightSystem::defaultLight;
LightShader LightSystem::lightShader;

void LightSystem::InitLights()
{
	// Use the unified shader loaded by your existing shader loader.
	lightShader.shader = Shared::shaders["lighting"];

	// Retrieve the unified shader's uniform locations.
	lightShader.lightCountLoc =
		GetShaderLocation(lightShader.shader, "lightCount");

	lightShader.lightTypesLoc =
		GetShaderLocation(lightShader.shader, "lightTypes");

	lightShader.lightPositionsLoc =
		GetShaderLocation(lightShader.shader, "lightPositions");

	lightShader.lightDirectionsLoc =
		GetShaderLocation(lightShader.shader, "lightDirections");

	lightShader.lightColorsLoc =
		GetShaderLocation(lightShader.shader, "lightColors");

	lightShader.lightIntensitiesLoc =
		GetShaderLocation(lightShader.shader, "lightIntensities");

	lightShader.lightRangesLoc =
		GetShaderLocation(lightShader.shader, "lightRanges");

	lightShader.spotAnglesLoc =
		GetShaderLocation(lightShader.shader, "spotAngles");

	lightShader.baseColorLoc =
		GetShaderLocation(lightShader.shader, "baseColor");
}

void LightSystem::Create(Vector3 pos, LIGHT_TYPE type)
{
	Light l;
	l.position = pos;
	l.type = type;
	Shared::lights.push_back(l);
}

void LightSystem::DrawLightEX(bool& camMode)
{
	if (camMode == false) return;//PLayer cam

	for (auto& light : Shared::lights) {
		if(light.enabled)
			DrawSphereWires(light.position, 5.0f,8,8, BLUE);
		else
			DrawSphereWires(light.position, 5.0f, 8, 8, RED);
	}
}

void LightSystem::DuplicateSelectedLight(Light* closestLight)
{
	if (closestLight) {
		Light newLight = *closestLight;
		newLight.position = Vector3Add(newLight.position, { 10,0,0 }); // Offset the duplicated light's position
		Shared::lights.push_back(newLight);
	}
}

void LightSystem::Update(bool& camMode, bool& freeCamToggle)
{
	if (camMode == false) return;//PLayer cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);
	
	float closestDistance = INFINITY;
	Light* closestLight = nullptr;
	//Select
	for (int i = 0; i < Shared::lights.size(); i++) {
		Light& light = Shared::lights[i];
		RayCollision hit = GetRayCollisionSphere(ray, light.position, 5);

		//fIND THE CLOSEST lIGHT
		if (hit.hit && hit.distance < closestDistance) {
			closestDistance = hit.distance;
			closestLight = &light;
		}
		
	}

	//If closest Light is found, check for selection or deletion
	if (closestLight) {
		//Select
		if (IsKeyPressed(KEY_C)) {
			selectedLight = closestLight;
			freeCamToggle = false;
		}

		//Delete the closest light if right mouse button is pressed
		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

			//Index found using pointer subtraction
			auto index = closestLight - Shared::lights.data();

			selectedLight = nullptr;
			freeCamToggle = true;

			Shared::lights.erase(Shared::lights.begin() + index);
		}

		//Duplicate Light
		if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_X)) {
			DuplicateSelectedLight(closestLight);
		}
	}

	//Update
	if (selectedLight != nullptr) {
		Vector2 mouseDelta = GetMouseDelta();

		Vector3 perp = Vector3CrossProduct(Shared::free.forward, { 0,1,0 });
		Vector3 delta = Vector3Normalize(Vector3Add(Vector3Scale(perp, GetMouseDelta().x), Vector3Scale(Shared::free.forward, -GetMouseDelta().y)));
		static float moveAmount = 4.0f;

		moveAmount += GetMouseWheelMove() * 0.5f;
		moveAmount = Clamp(moveAmount, 0.5f, 10.0f);

		Vector3 move = Vector3Scale(delta, moveAmount);

		if (IsKeyPressed(KEY_UP))
			move.y = 4.0f;
		else if (IsKeyPressed(KEY_DOWN))
			move.y = -4.0f;
		else
			move.y = 0.0f;

		//----Updating Y-----
		selectedLight->position = Vector3Add(selectedLight->position, { 0,move.y, 0 });
		//----Updateing XZ---
		selectedLight->position = Vector3Add(selectedLight->position, {move.x,0,move.z});

	}

	//Deselect
	if (IsKeyReleased(KEY_C)) {
		selectedLight = nullptr;
		freeCamToggle = true;
	}
}

void LightSystem::UpdateShaderValues(LightShader& lightShader)
{
	constexpr int MAX_LIGHTS = 32;

	int lightTypes[MAX_LIGHTS] = {};
	float lightPositions[MAX_LIGHTS * 3] = {};
	float lightDirections[MAX_LIGHTS * 3] = {};
	float lightColors[MAX_LIGHTS * 4] = {};
	float lightIntensities[MAX_LIGHTS] = {};
	float lightRanges[MAX_LIGHTS] = {};
	float spotAngles[MAX_LIGHTS] = {};

	int count = 0;

	for (const Light& light : Shared::lights)
	{
		if (!light.enabled || count >= MAX_LIGHTS)//Because of the default
			continue;

		lightTypes[count] = static_cast<int>(light.type);

		lightPositions[count * 3 + 0] = light.position.x;
		lightPositions[count * 3 + 1] = light.position.y;
		lightPositions[count * 3 + 2] = light.position.z;

		Vector3 dirToUse = light.dir;
		if (light.type == DIRECTIONAL_LIGHT)
			dirToUse = Vector3Scale(Vector3Normalize(light.position), -1.0f);

		lightDirections[count * 3 + 0] = dirToUse.x;
		lightDirections[count * 3 + 1] = dirToUse.y;
		lightDirections[count * 3 + 2] = dirToUse.z;

		Vector4 color = ColorNormalize(light.color);
		lightColors[count * 4 + 0] = color.x;
		lightColors[count * 4 + 1] = color.y;
		lightColors[count * 4 + 2] = color.z;
		lightColors[count * 4 + 3] = color.w;

		lightIntensities[count] = light.intensity;
		lightRanges[count] = light.range;
		spotAngles[count] = light.spotAngle;

		++count;
	}

	//Default Directional Light
	if (LightSystem::defaultLight.enabled) {

		auto& light = LightSystem::defaultLight;

		lightTypes[count] = static_cast<int>(LightSystem::defaultLight.type);

		lightPositions[count * 3 + 0] = light.position.x;
		lightPositions[count * 3 + 1] = light.position.y;
		lightPositions[count * 3 + 2] = light.position.z;

		Vector3 dirToUse = Vector3Scale(Vector3Normalize(light.position), -1.0f);

		lightDirections[count * 3 + 0] = dirToUse.x;
		lightDirections[count * 3 + 1] = dirToUse.y;
		lightDirections[count * 3 + 2] = dirToUse.z;

		Vector4 color = ColorNormalize(light.color);
		lightColors[count * 4 + 0] = color.x;
		lightColors[count * 4 + 1] = color.y;
		lightColors[count * 4 + 2] = color.z;
		lightColors[count * 4 + 3] = color.w;

		lightIntensities[count] = light.intensity;
		lightRanges[count] = light.range;
		spotAngles[count] = light.spotAngle;

		++count;
	}


	SetShaderValue(lightShader.shader, lightShader.lightCountLoc, &count, SHADER_UNIFORM_INT);

	if (count > 0)
	{
		SetShaderValueV(lightShader.shader, lightShader.lightTypesLoc, lightTypes, SHADER_UNIFORM_INT, count);
		SetShaderValueV(lightShader.shader, lightShader.lightPositionsLoc, lightPositions, SHADER_UNIFORM_VEC3, count);
		SetShaderValueV(lightShader.shader, lightShader.lightDirectionsLoc, lightDirections, SHADER_UNIFORM_VEC3, count);
		SetShaderValueV(lightShader.shader, lightShader.lightColorsLoc, lightColors, SHADER_UNIFORM_VEC4, count);
		SetShaderValueV(lightShader.shader, lightShader.lightIntensitiesLoc, lightIntensities, SHADER_UNIFORM_FLOAT, count);
		SetShaderValueV(lightShader.shader, lightShader.lightRangesLoc, lightRanges, SHADER_UNIFORM_FLOAT, count);
		SetShaderValueV(lightShader.shader, lightShader.spotAnglesLoc, spotAngles, SHADER_UNIFORM_FLOAT, count);
	}

	Vector4 baseColor = ColorNormalize(WHITE);
	SetShaderValue(lightShader.shader, lightShader.baseColorLoc, &baseColor, SHADER_UNIFORM_VEC4);
}
