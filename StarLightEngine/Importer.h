#pragma once
class Importer
{

public:
	static void InitTextures();
	static void InitModels();
	static void InitModelAnimationNames();
	static void InitSounds();
	static void InitMusic();
	static void InitShaders();


	static void UnloadTextures();
	static void UnloadModels();
	static void UnloadSounds();
	static void UnloadMusic();
	static void UnloadShaders();
};

