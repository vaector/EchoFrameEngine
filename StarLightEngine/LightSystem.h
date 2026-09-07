#pragma once
#include <vector>
#include "raylib.h"


enum LIGHT_TYPE {
	POINT_LIGHT,
	SPOT_LIGHT,
	DIRECTIONAL_LIGHT
};

struct LightShader{ // The uniforms for all light shaders
	Shader shader;

	int lightPositionLoc = -1;
	int lightDirectionLoc = -1;
	int lightColorLoc = -1;
	int lightIntensityLoc = -1;
	int lightRangeLoc = -1;
	int spotAngleLoc = -1;
	int baseColorLoc = -1;
};

struct Light{
	Vector3 position = {0};
	Vector3 dir = { 0,-1,0 };

	LIGHT_TYPE type = POINT_LIGHT;

	float intensity = 2.0f;
	float range = 200.0f;
	float spotAngle = 45.0f;

	Color color = WHITE;

	bool enabled = true;

};
static class LightSystem
{	

public:
	static Light* selectedLight;

	//Light Shaders
	static LightShader pointShader;
	static LightShader spotShader;
	static LightShader directionalShader;

	static void InitLights();//Init Lights
	static void Create(Vector3 pos, LIGHT_TYPE type);//Create Lights (position, lightType).
	static void DrawLightEX(bool& camMode);

	static void Update(bool& camMode, bool& freeCamToggle);//camMode -> bool in realEditor if false then we are using the free Cam(Editor Camera).
	static void UpdateShaderValues(LightShader& lightShader, Light& light);
};

