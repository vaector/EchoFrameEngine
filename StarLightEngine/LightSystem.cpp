#include "LightSystem.h"
#include "Shared.h"
#include "raymath.h"

Light* LightSystem::selectedLight = nullptr;

LightShader LightSystem::pointShader;
LightShader LightSystem::spotShader;
LightShader LightSystem::directionalShader;

void LightSystem::InitLights()
{
	// Point shader uniforms
	pointShader.shader = Shared::shaders["point"];
	spotShader.shader = Shared::shaders["spot"];
	directionalShader.shader = Shared::shaders["directional"];


	// Point
	pointShader.lightPositionLoc =
		GetShaderLocation(pointShader.shader, "lightPosition");

	pointShader.lightColorLoc =
		GetShaderLocation(pointShader.shader, "lightColor");

	pointShader.lightIntensityLoc =
		GetShaderLocation(pointShader.shader, "lightIntensity");

	pointShader.lightRangeLoc =
		GetShaderLocation(pointShader.shader, "lightRange");

	pointShader.baseColorLoc =
		GetShaderLocation(pointShader.shader, "baseColor");


	// Directional
	directionalShader.lightDirectionLoc =
		GetShaderLocation(directionalShader.shader, "lightDirection");

	directionalShader.lightColorLoc =
		GetShaderLocation(directionalShader.shader, "lightColor");

	directionalShader.lightIntensityLoc =
		GetShaderLocation(directionalShader.shader, "lightIntensity");

	directionalShader.baseColorLoc =
		GetShaderLocation(directionalShader.shader, "baseColor");

	// Spot
	spotShader.lightPositionLoc =
		GetShaderLocation(spotShader.shader, "lightPosition");

	spotShader.lightDirectionLoc =
		GetShaderLocation(spotShader.shader, "lightDirection");

	spotShader.lightColorLoc =
		GetShaderLocation(spotShader.shader, "lightColor");

	spotShader.lightIntensityLoc =
		GetShaderLocation(spotShader.shader, "lightIntensity");

	spotShader.lightRangeLoc =
		GetShaderLocation(spotShader.shader, "lightRange");

	spotShader.spotAngleLoc =
		GetShaderLocation(spotShader.shader, "spotAngle");

	spotShader.baseColorLoc =
		GetShaderLocation(spotShader.shader, "baseColor");
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

void LightSystem::Update(bool& camMode, bool& freeCamToggle)
{
	if (camMode == false) return;//PLayer cam

	Ray ray = GetScreenToWorldRay({ (float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2 }, Shared::free.cam);
	
	//Select
	for (int i = 0; i < Shared::lights.size(); i++) {
		Light& light = Shared::lights[i];
		RayCollision hit = GetRayCollisionSphere(ray, light.position, 5);

		if (hit.hit) {
			if (IsKeyPressed(KEY_C)) {
				selectedLight = &light;
				freeCamToggle = false;
				break;
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				Shared::lights.erase(Shared::lights.begin() + i);
				LightSystem::selectedLight = nullptr;
			}
		}
	}

	//Update
	if (selectedLight != nullptr) {
		Vector2 mouseDelta = GetMouseDelta();

		Vector3 perp = Vector3CrossProduct(Shared::free.forward, { 0,1,0 });
		Vector3 delta = Vector3Normalize(Vector3Add(Vector3Scale(perp, GetMouseDelta().x), Vector3Scale(Shared::free.forward, -GetMouseDelta().y)));
		Vector3 move = Vector3Scale(delta, 4.0f);

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

void LightSystem::UpdateShaderValues(LightShader& lightShader, Light& light)
{
	SetShaderValue(
		lightShader.shader,
		lightShader.lightPositionLoc,
		&light.position,
		SHADER_UNIFORM_VEC3
	);

	Vector3 dirToUse = light.dir;
	if (light.type == DIRECTIONAL_LIGHT)
		dirToUse = Vector3Scale(Vector3Normalize(light.position), -1.0f);

	SetShaderValue(
		lightShader.shader,
		lightShader.lightDirectionLoc,
		&dirToUse,
		SHADER_UNIFORM_VEC3
	);

	Vector4 color = ColorNormalize(light.color);

	SetShaderValue(
		lightShader.shader,
		lightShader.lightColorLoc,
		&color,
		SHADER_UNIFORM_VEC4
	);

	SetShaderValue(
		lightShader.shader,
		lightShader.lightIntensityLoc,
		&light.intensity,
		SHADER_UNIFORM_FLOAT
	);

	SetShaderValue(
		lightShader.shader,
		lightShader.lightRangeLoc,
		&light.range,
		SHADER_UNIFORM_FLOAT
	);

	SetShaderValue(
		lightShader.shader,
		lightShader.spotAngleLoc,
		&light.spotAngle,
		SHADER_UNIFORM_FLOAT
	);

	Vector4 baseColor = ColorNormalize(WHITE);

	SetShaderValue(
		lightShader.shader,
		lightShader.baseColorLoc,
		&baseColor,
		SHADER_UNIFORM_VEC4
	);
}
