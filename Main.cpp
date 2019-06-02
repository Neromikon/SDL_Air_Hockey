#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "Game.h"


void main()
{
	if (Game::Init())
	{
		while (!Game::IsEnded())
		{
			Game::StartFrame();
			Game::ProcessEvents();
			Game::Update();
			Game::Render();
			Game::FinishFrame();
		}
	}

	Game::Exit();
}