#include "Game.h"
#include <iostream>
#include "Shared.h"
#include "Hand.h"

Hand hand;

void Game::Start()
{
	//Start the game, initialize all the necessary components and resources.

	hand.CreateHand("handGun");
}

void Game::Draw()
{
	//Draw the game scene, render all the entities and UI elements.
	hand.DrawHand();

}

void Game::Update()
{
	//Update the game state, handle input, update entities, and render the scene.

	hand.UpdateHand();

}
