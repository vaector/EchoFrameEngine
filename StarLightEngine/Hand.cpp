#include "Hand.h"

void Hand::CreateHand(std::string name)
{
	textureName = name;

}

void Hand::UpdateHand()
{
	//Hand Logic Here

	

}

void Hand::DrawHand()
{
	//Render Hand
	DrawTexture(Shared::textures[textureName], GetScreenWidth() - 200, GetScreenHeight() - 200, WHITE);
}
