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
	float width = GetScreenWidth()-(Shared::textures[textureName].width)*1.75;
	float height = GetScreenHeight()-(Shared::textures[textureName].height)*1.75;
	DrawTextureEx(Shared::textures[textureName], {width, height}, 0, 1.75, WHITE);
}
