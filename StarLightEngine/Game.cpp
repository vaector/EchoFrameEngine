#include "Game.h"
#include "Shared.h"
#include "GameShared.h"


void Game::Start()
{
	//Start the game, initialize all the necessary components and resources.

	GameShared::hand.CreateHand("handGun");
}

void Game::DrawReal()
{
	//Draw the game scene, render all the entities and UI elements.
	GameShared::hand.DrawCable();

}

void Game::DrawUi()
{
	//Draw the game UI elements, such as health bars, score, etc.
	GameShared::hand.DrawHand();
}

void Game::Update()
{
	//Update the game state, handle input, update entities.

	GameShared::hand.UpdateHand();

}
