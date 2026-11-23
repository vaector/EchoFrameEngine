#include "Game.h"
#include <iostream>
#include "Shared.h"

void Game::Start()
{
	std::cout << "Game started!" << std::endl;
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
	std::cout << "Game Running!" << std::endl;
	//Update the game state, handle input, update entities, and render the scene.

	hand.UpdateHand();

}
