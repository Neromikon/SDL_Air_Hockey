#include "Game.h"

#include <iostream>
#include <string>

#include <SDL_image.h>

#include "Entity.h"


namespace
{
	const Uint32 k_initFlags = SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO;
	const Uint32 k_windowFlags = SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN;

	const size_t k_maxEntities = 10;

	const float k_stickMovePower = 8.75f;
	const float k_wallsVelocityConsumption = 0.25f;

	const float k_wallsWidth = 0.05f;
	const float k_gateWidth = 0.3f;

	const float k_stickRadius = 0.075f;
	const float k_stickFriction = 3.15f;
	const float k_stickMass = 5.0f;

	const float k_puckRadius = 0.025f;
	const float k_puckFriction = 0.11f;
	const float k_puckMass = 1.25f;

	const SDL_Color k_textColor = { 0, 0, 0, 255 };

	const unsigned int k_stickCollisionMask = Entity::PUCK_LAYER | Entity::WALL_LAYER;
	const unsigned int k_puckCollisionMask = Entity::STICK_LAYER | Entity::WALL_LAYER | Entity::GATE_LAYER;
	const unsigned int k_wallCollisionMask = Entity::STICK_LAYER | Entity::PUCK_LAYER;
	const unsigned int k_gateCollisionMask = Entity::PUCK_LAYER;
}


float Game::deltaTime;

SDL_DisplayMode Game::displayMode;

glm::ivec2 Game::windowPosition;
glm::ivec2 Game::windowSize;
glm::ivec2 Game::windowCenter;
float Game::windowRatio;
float Game::reverseWindowRatio;

bool Game::s_isEnded = false;
Uint32 Game::s_frameStartMoment = 0;

SDL_Window* Game::s_window = nullptr;
SDL_Renderer* Game::s_renderer = nullptr;
SDL_Rect Game::s_scoreRect1;
SDL_Rect Game::s_scoreRect2;

float Game::s_desiredFPS = 60.0f; // Hertz
Uint32 Game::s_desiredFramePeriod; // milliseconds

KeyboardController Game::s_player;
AIController Game::s_bot;

Controller* Game::s_player1 = nullptr;
Controller* Game::s_player2 = nullptr;
unsigned int Game::s_count1 = 0;
unsigned int Game::s_count2 = 0;

std::vector<Entity> Game::s_entities;
std::vector<line> Game::s_borders;

SDL_Texture* Game::s_backgroundTexture = nullptr;
SDL_Texture* Game::s_puckTexture = nullptr;
SDL_Texture* Game::s_stickTexture1 = nullptr;
SDL_Texture* Game::s_stickTexture2 = nullptr;
SDL_Texture* Game::s_stickAnimationSheet1 = nullptr;
SDL_Texture* Game::s_stickAnimationSheet2 = nullptr;
SDL_Texture* Game::s_gateTexture = nullptr;
SDL_Texture* Game::s_gateAnimationSheet = nullptr;

TTF_Font* Game::s_font = nullptr;
SDL_Texture* Game::s_scoreTexture1 = nullptr;
SDL_Texture* Game::s_scoreTexture2 = nullptr;

Mix_Music* Game::s_puckCollidesWallSound = nullptr;
Mix_Music* Game::s_puckCollidesStickSound = nullptr;
Mix_Music* Game::s_puckEntersGateSound = nullptr;

Entity* Game::s_stick1 = nullptr;
Entity* Game::s_stick2 = nullptr;
Entity* Game::s_puck = nullptr;

circle Game::s_puckSpawner;


const std::vector<Resource<SDL_Texture*>> Game::k_textureResources =
{
	Resource<SDL_Texture*>(Game::s_backgroundTexture, "Assets/Background.png"),
	Resource<SDL_Texture*>(Game::s_puckTexture, "Assets/Puck.png"),
	Resource<SDL_Texture*>(Game::s_stickTexture1, "Assets/StickP1.png"),
	Resource<SDL_Texture*>(Game::s_stickTexture2, "Assets/StickP2.png"),
	Resource<SDL_Texture*>(Game::s_stickAnimationSheet1, "Assets/StickP1Animation.png"),
	Resource<SDL_Texture*>(Game::s_stickAnimationSheet2, "Assets/StickP2Animation.png"),
	Resource<SDL_Texture*>(Game::s_gateTexture, "Assets/Gate.png"),
	Resource<SDL_Texture*>(Game::s_gateAnimationSheet, "Assets/GateAnimation.png"),
};

const std::vector<Resource<Mix_Music*>> Game::k_soundResources =
{
	Resource<Mix_Music*>(Game::s_puckCollidesWallSound, "Assets/Pong.mp3"),
	Resource<Mix_Music*>(Game::s_puckCollidesStickSound, "Assets/PongLow.mp3"),
	Resource<Mix_Music*>(Game::s_puckEntersGateSound, "Assets/RoundEnd.mp3")
};

const Resource<TTF_Font*> Game::k_fontResource(Game::s_font, "Assets/consolas.ttf");


bool Game::Init()
{
	if(!InitCore()) { return false; }
	if(!InitTextures()) { return false; }
	if(!InitAudio()) { return false; }
	if(!InitPlayground()) { return false; }
	if(!InitWalls()) { return false; }
	if (!InitInterface()) { return false; }

	Restart();

	return true;
}


void Game::Exit()
{
	for (auto &texture : k_textureResources)
	{
		if (texture == nullptr) { continue; }
		SDL_DestroyTexture(*(texture.data));
	}

	for (auto &sound : k_soundResources)
	{
		if (sound == nullptr) { continue; }
		Mix_FreeMusic(*(sound.data));
	}

	TTF_CloseFont(s_font);
	TTF_Quit();

	SDL_DestroyRenderer(s_renderer);
	SDL_DestroyWindow(s_window);

	SDL_Quit();
}


void Game::StartFrame()
{
	s_frameStartMoment = SDL_GetTicks();
}


void Game::ProcessEvents()
{
	SDL_Event sdlEvent;
	if (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
			case SDL_KEYDOWN:
			{
				if (sdlEvent.key.keysym.scancode == SDL_Scancode::SDL_SCANCODE_ESCAPE)
				{
					s_isEnded = true;
				}

				s_player.OnKeyboardDown(sdlEvent.key.keysym.scancode);

				break;
			}

			case SDL_KEYUP:
			{
				s_player.OnKeyboardUp(sdlEvent.key.keysym.scancode);

				break;
			}

			case SDL_QUIT:
			{
				s_isEnded = true;
				break;
			}
		}
	}
}


void Game::Update()
{
	deltaTime = 0.001f * static_cast<float>(s_desiredFramePeriod); //fixed update is used so delta time does not change each frame

	if (!s_puck->IsEnabled())
	{
		if (IsPuckSpawnerFree())
		{
			s_puck->SetEnabled(true);
		}
	}

	s_player1->Update();
	s_player2->Update();

	CheckCollisions();

	for (int i = 0; i < s_entities.size(); i++)
	{
		if (!s_entities[i].IsEnabled()) { continue; }

		s_entities[i].Update();
	}
}


void Game::Render()
{
	SDL_RenderCopy(s_renderer, s_backgroundTexture, NULL, NULL);

	for (int i = 0; i < s_entities.size(); i++)
	{
		if (!s_entities[i].IsEnabled()) { continue; }

		s_entities[i].Draw();
	}

	for (int i = 0; i < s_borders.size(); i++)
	{
		int x1 = s_borders[i].point1.x * windowSize.x;
		int y1 = (1.0f - s_borders[i].point1.y * windowRatio) * windowSize.y;
		int x2 = s_borders[i].point2.x * windowSize.x;
		int y2 = (1.0f - s_borders[i].point2.y * windowRatio) * windowSize.y;
		SDL_RenderDrawLine(s_renderer, x1, y1, x2, y2);
	}

	SDL_RenderCopy(s_renderer, s_scoreTexture1, nullptr, &s_scoreRect1);
	SDL_RenderCopy(s_renderer, s_scoreTexture2, nullptr, &s_scoreRect2);

	SDL_RenderPresent(s_renderer);
}


void Game::FinishFrame()
{
	const Uint32 frameDuration = SDL_GetTicks() - s_frameStartMoment;

	if (frameDuration < Game::s_desiredFramePeriod)
	{
		SDL_Delay(Game::s_desiredFramePeriod - frameDuration);
	}
}


void Game::Restart()
{
	s_count1 = 0;
	s_count2 = 0;

	SDL_Surface* surface;

	surface = TTF_RenderText_Blended(s_font, std::to_string(s_count1).c_str(), k_textColor);
	s_scoreTexture1 = SDL_CreateTextureFromSurface(s_renderer, surface);
	SDL_FreeSurface(surface);

	surface = TTF_RenderText_Blended(s_font, std::to_string(s_count2).c_str(), k_textColor);
	s_scoreTexture2 = SDL_CreateTextureFromSurface(s_renderer, surface);
	SDL_FreeSurface(surface);

	s_stick1->SetAnchoredPosition(glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.25f));
	s_stick2->SetAnchoredPosition(glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.75f));

	NextRound();
}


void Game::NextRound()
{
	s_puck->SetAnchoredPosition(glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.5f));
	s_puck->SetVelocity(glm::vec2(0.0f, 0.0f));

	if (IsPuckSpawnerFree())
	{
		s_puck->SetEnabled(true);
	}
	else
	{
		s_puck->SetEnabled(false);
	}
}


Entity* Game::CreateStick(SDL_Texture *texture, SDL_Texture *animationSheet)
{
	Entity &entity = AddEntity();

	entity.SetLayerMask(Entity::STICK_LAYER);
	entity.SetCollisionMask(k_stickCollisionMask);
	entity.SetRenderer(s_renderer);
	entity.SetMass(k_stickMass);
	entity.SetShape(shape::CIRCLE);
	entity.SetSize(glm::vec2(k_stickRadius * 2.0f));
	entity.SetFrinction(k_stickFriction);
	entity.OnCollision(OnStickCollision);

	entity.AddAnimation("Idle", Animation::CreateSingleFrame(texture));

	{
		const std::vector<int> animFrames ={ 0, 1, 2, 3, 3, 3, 2, 1, 0 };
		Animation &addedAnimation = entity.AddAnimation("Blink", Animation::CreateFromSpriteSheet2x2(animationSheet, animFrames));
		addedAnimation.SetNextState("Idle");
		addedAnimation.SetDuration(0.25f);
	}

	return &entity;
}


Entity* Game::CreatePuck()
{
	Entity &entity = AddEntity();

	entity.SetLayerMask(Entity::PUCK_LAYER);
	entity.SetCollisionMask(k_puckCollisionMask);
	entity.SetRenderer(s_renderer);
	entity.SetMass(k_puckMass);
	entity.SetShape(shape::CIRCLE);
	entity.SetSize(glm::vec2(k_puckRadius * 2.0f));
	entity.SetFrinction(k_puckFriction);
	entity.OnCollisionWithLayer(OnPuckCollision);

	entity.AddAnimation("Idle", Animation::CreateSingleFrame(s_puckTexture));

	return &entity;
}


Entity* Game::CreateGate()
{
	Entity &entity = AddEntity();

	entity.SetLayerMask(Entity::GATE_LAYER);
	entity.SetCollisionMask(k_gateCollisionMask);
	entity.SetRenderer(s_renderer);
	entity.SetMass(0.0f);
	entity.SetShape(shape::RECTANGLE);
	entity.SetStatic(true);

	entity.AddAnimation("Idle", Animation::CreateSingleFrame(s_gateTexture));

	{
		const std::vector<int> animFrames ={ 0, 1, 2, 3 };
		Animation &addedAnimation = entity.AddAnimation("Score", Animation::CreateFromSpriteSheet2x2(s_gateAnimationSheet, animFrames));
		addedAnimation.SetNextState("Idle");
		addedAnimation.SetDuration(0.3f);
	}

	return &entity;
}


bool Game::InitCore()
{
	if (SDL_Init(k_initFlags) != 0)
	{
		std::cerr << "Failed to init SDL: " << SDL_GetError() << "\n";
		return false;
	}

	if (TTF_Init() != 0)
	{
		std::cerr << "Failed to init SDL fonts: " << SDL_GetError() << "\n";
		return false;
	}

	if (SDL_GetCurrentDisplayMode(0, &Game::displayMode) != 0)
	{
		std::cerr << "SDL error: " << SDL_GetError() << "\n";
		return false;
	}

	windowPosition = glm::ivec2(Game::displayMode.w * 0.5 - Game::displayMode.h * 0.3, Game::displayMode.h * 0.1);
	windowSize = glm::ivec2(Game::displayMode.h * 0.6, Game::displayMode.h * 0.8);
	windowCenter = glm::ivec2(windowSize.x / 2, windowSize.y / 2);
	windowRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
	reverseWindowRatio = 1.0f / windowRatio;

	s_window = SDL_CreateWindow("Air hockey", windowPosition.x, windowPosition.y, windowSize.x, windowSize.y, k_windowFlags);
	if (s_window == nullptr)
	{
		std::cerr << "Failed to create SDL window: " << SDL_GetError() << "\n";
		return false;
	}

	s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED);
	if (s_renderer == nullptr)
	{
		std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << "\n";
		return false;
	}

	s_desiredFramePeriod = 1000.0f / s_desiredFPS;

	return true;
}


bool Game::InitTextures()
{
	SDL_Surface* tempSurface;

	for (auto& resource : k_textureResources)
	{
		tempSurface = IMG_Load(resource.file.c_str());

		if (tempSurface == nullptr)
		{
			std::cerr << "Failed to load image " << resource.file << ": " << IMG_GetError() << "\n";
			return false;
		}

		resource = SDL_CreateTextureFromSurface(s_renderer, tempSurface);

		SDL_FreeSurface(tempSurface);

		if (*(resource.data) == nullptr)
		{
			std::cerr << "Failed to create texture from image " << resource.file << ": " << SDL_GetError() << "\n";
			return false;
		}
	}

	return true;
}


bool Game::InitAudio()
{
	if (Mix_Init(MIX_INIT_MP3) == 0)
	{
		std::cerr << "Failed to initialize SDL audio mixer: " << Mix_GetError() << "\n";
		return false;
	}

	if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640) != 0)
	{
		std::cerr << "Failed to open SDL audio mixer: " << Mix_GetError() << "\n";
		return false;
	}

	for (auto& resource : k_soundResources)
	{
		resource = Mix_LoadMUS(resource.file.c_str());
		if (resource == nullptr)
		{
			std::cerr << "Failed to load sound " << resource.file << ": " << Mix_GetError() << "\n";
			return false;
		}
	}

	return true;
}


bool Game::InitPlayground()
{
	s_entities.reserve(k_maxEntities);

	s_stick1 = CreateStick(s_stickTexture1, s_stickAnimationSheet1);

	s_stick2 = CreateStick(s_stickTexture2, s_stickAnimationSheet2);

	s_puck = CreatePuck();

	Entity *wall;

	Entity *gate1 = CreateGate();
	gate1->OnCollision(OnPlayer2Score);
	gate1->SetSize(glm::vec2(k_gateWidth, k_wallsWidth * 0.5f));
	gate1->SetAnchoredPosition(glm::vec2(0.0f, k_wallsWidth * 0.25f), glm::vec2(0.5f, 0.0f));

	Entity *gate2 = CreateGate();
	gate2->OnCollision(OnPlayer1Score);
	gate2->SetSize(glm::vec2(k_gateWidth, k_wallsWidth * 0.5f));
	gate2->SetAnchoredPosition(glm::vec2(0.0f, -k_wallsWidth * 0.25f), glm::vec2(0.5f, 1.0f));

	s_player1 = &s_player;
	s_player2 = &s_bot;

	s_player1->SetControlTarget(s_stick1);
	s_player1->SetMoveForce(k_stickMovePower);
	s_player1->SetArea(rectangle(glm::vec2(0.5f, 0.25f * reverseWindowRatio), glm::vec2(1.0f, reverseWindowRatio * 0.24f)));

	s_player2->SetControlTarget(s_stick2);
	s_player2->SetMoveForce(k_stickMovePower);
	s_player2->SetArea(rectangle(glm::vec2(0.5f, 0.75f * reverseWindowRatio), glm::vec2(1.0f, reverseWindowRatio * 0.27f)));

	SDL_assert(gate2->GetShape().m_type == shape::RECTANGLE);
	s_bot.SetPuck(s_puck);
	s_bot.SetOwnGateRectangle(gate2->GetShape().m_data.m_rectangle);
	s_bot.SetOpponentGateRectangle(gate1->GetShape().m_data.m_rectangle);

	s_puckSpawner.position = glm::vec2(0.5f, reverseWindowRatio * 0.5f);
	s_puckSpawner.radius = k_puckRadius * 2.5f;

	return true;
}


bool Game::InitWalls()
{
	const std::vector<glm::vec2> borderStrip =
	{
		glm::vec2(k_wallsWidth, k_wallsWidth),
		glm::vec2(k_wallsWidth, reverseWindowRatio - k_wallsWidth),
		glm::vec2(0.5f - k_gateWidth * 0.5f, reverseWindowRatio - k_wallsWidth),
		glm::vec2(0.5f - k_gateWidth * 0.5f, reverseWindowRatio),
		glm::vec2(0.5f + k_gateWidth * 0.5f, reverseWindowRatio),
		glm::vec2(0.5f + k_gateWidth * 0.5f, reverseWindowRatio - k_wallsWidth),
		glm::vec2(1.0f - k_wallsWidth, reverseWindowRatio - k_wallsWidth),
		glm::vec2(1.0f - k_wallsWidth, k_wallsWidth),
		glm::vec2(0.5f + k_gateWidth * 0.5f, k_wallsWidth),
		glm::vec2(0.5f + k_gateWidth * 0.5f, 0.0f),
		glm::vec2(0.5f - k_gateWidth * 0.5f, 0.0f),
		glm::vec2(0.5f - k_gateWidth * 0.5f, k_wallsWidth),
	};

	for (int i = 0; i < borderStrip.size() - 1; i++)
	{
		s_borders.push_back(line(borderStrip[i], borderStrip[i+1]));
	}

	s_borders.push_back(line(borderStrip[borderStrip.size() - 1], borderStrip[0]));

	return true;
}


bool Game::InitInterface()
{
	k_fontResource = TTF_OpenFont(k_fontResource.file.c_str(), 18);
	if (k_fontResource == nullptr)
	{
		std::cerr << "Failed to load font " << k_fontResource.file << ": " << TTF_GetError() << "\n";
		return false;
	}

	s_scoreRect1.x = windowSize.x - 50;
	s_scoreRect1.y = windowSize.y - 25;
	s_scoreRect1.w = 50;
	s_scoreRect1.h = 25;

	s_scoreRect2.x = windowSize.x - 50;
	s_scoreRect2.y = 0;
	s_scoreRect2.w = 50;
	s_scoreRect2.h = 25;

	return true;
}


Entity& Game::AddEntity()
{
	SDL_assert(s_entities.size() < k_maxEntities);

	s_entities.emplace_back();

	return s_entities.back();
}


void Game::CheckCollisions()
{
	for (int i = 0; i < s_entities.size(); i++)
	{
		if (!s_entities[i].IsEnabled()) { continue; }

		for (int j = i + 1; j < s_entities.size(); j++)
		{
			if (!s_entities[j].IsEnabled()) { continue; }

			if (s_entities[i].Contact(s_entities[j]))
			{
				s_entities[i].Collide(s_entities[j]);
			}
		}

		for (int j = 0; j <s_borders.size(); j++)
		{
			if (!s_entities[i].CanCollideWith(Entity::WALL_LAYER)) { continue; }

			if (s_entities[i].Contact(s_borders[j], k_wallCollisionMask))
			{
				s_entities[i].ReflectFrom(s_borders[j], Entity::WALL_LAYER, k_wallsVelocityConsumption);
			}
		}
	}
}


bool Game::IsPuckSpawnerFree()
{
	for (int i = 0; i < s_entities.size(); i++)
	{
		if (!s_entities[i].IsEnabled()) { continue; }

		if (s_entities[i].Contact(s_puckSpawner)) { return false; }
	}

	return true;
}


void Game::OnPlayer1Score(Entity* entity1, Entity* entity2)
{
	s_count1++;

	Mix_PlayMusic(s_puckEntersGateSound, 1);

	SDL_Surface *surface = TTF_RenderText_Blended(s_font, std::to_string(s_count1).c_str(), k_textColor);

	s_scoreTexture1 = SDL_CreateTextureFromSurface(s_renderer, surface);

	SDL_FreeSurface(surface);

	entity1->Play("Score");

	NextRound();
}


void Game::OnPlayer2Score(Entity* entity1, Entity* entity2)
{
	s_count2++;

	Mix_PlayMusic(s_puckEntersGateSound, 1);

	SDL_Surface *surface = TTF_RenderText_Blended(s_font, std::to_string(s_count2).c_str(), k_textColor);

	s_scoreTexture2 = SDL_CreateTextureFromSurface(s_renderer, surface);

	SDL_FreeSurface(surface);

	entity1->Play("Score");

	NextRound();
}


void Game::OnStickCollision(Entity* entity1, Entity* entity2)
{
	if (entity2 == s_puck)
	{
		entity1->PlayIfNotPlaying("Blink");
	}
}


void Game::OnPuckCollision(Entity* entity, Entity::mask_t layerMask)
{
	switch (layerMask)
	{
		case Entity::WALL_LAYER: Mix_PlayMusic(s_puckCollidesWallSound, 1); break;
		case Entity::STICK_LAYER: Mix_PlayMusic(s_puckCollidesStickSound, 1); break;
	}
}