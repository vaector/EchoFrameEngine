#include "Hand.h"

void Hand::CreateHand(std::string name)
{
	textureName = name;

}

void Hand::UpdateHand()
{
	//Hand Logic Here

	//Render Hand
	DrawTexture(Shared::textures[textureName], GetScreenWidth()-200, GetScreenHeight()-200, WHITE);

}
