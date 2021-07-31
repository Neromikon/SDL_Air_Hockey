#pragma once

#include <vector>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <glm/glm.hpp>

#include "Controller.h"
#include "Entity.h"
#include "Resource.h"
#include "Event.h"


class Game //static
{
public:
	static float deltaTime; //seconds
	static SDL_DisplayMode displayMode;
	static glm::ivec2 windowPosition;
	static glm::ivec2 windowSize;
	static glm::ivec2 windowCenter;
	static float windowRatio;
	static float reverseWindowRatio;

	static bool Init();
	static void Exit();

	static void StartFrame();
	static void ProcessEvents();
	static void Update();
	static void Render();
	static void FinishFrame();

	static void Restart();

	static Entity* CreateStick(SDL_Texture *texture, SDL_Texture *animationSheet);
	static Entity* CreatePuck();
	static Entity* CreateGate();

	inline static bool IsEnded() { return s_isEnded; }

	static Event<void()> s_onNextRound;
	static Event<void()> s_onPuckCollision;

private:
	static bool s_isEnded;
	static Uint32 s_frameStartMoment;

	static SDL_Window* s_window;
	static SDL_Renderer* s_renderer;
	static SDL_Rect s_scoreRect1;
	static SDL_Rect s_scoreRect2;

	static float s_desiredFPS;
	static Uint32 s_desiredFramePeriod;

	static KeyboardController s_player;
	static AIController s_bot;

	static Controller *s_player1, *s_player2;
	static unsigned int s_count1, s_count2;

	static std::vector<Entity> s_entities;
	static std::vector<line> s_borders;

	static const std::vector<Resource<SDL_Texture*>> k_textureResources;
	static const std::vector<Resource<Mix_Music*>> k_soundResources;
	static const Resource<TTF_Font*> k_fontResource;

	static SDL_Texture* s_backgroundTexture;
	static SDL_Texture* s_puckTexture;
	static SDL_Texture* s_stickTexture1;
	static SDL_Texture* s_stickTexture2;
	static SDL_Texture* s_stickAnimationSheet1;
	static SDL_Texture* s_stickAnimationSheet2;
	static SDL_Texture* s_gateTexture;
	static SDL_Texture* s_gateAnimationSheet;

	static TTF_Font* s_font;
	static SDL_Texture* s_scoreTexture1;
	static SDL_Texture* s_scoreTexture2;

	static Mix_Music *s_puckCollidesWallSound;
	static Mix_Music *s_puckCollidesStickSound;
	static Mix_Music *s_puckEntersGateSound;
	static Mix_Music *s_scoreResetSound;

	static Entity *s_stick1, *s_stick2, *s_puck;

	static circle s_puckSpawner;


	static bool InitCore();
	static bool InitTextures();
	static bool InitAudio();
	static bool InitPlayground();
	static bool InitWalls();
	static bool InitInterface();

	static Entity& AddEntity();

	static void UpdatePuck();
	static void UpdatePlayers();
	static void UpdatePhysics();
	static void UpdateEntities();

	static void RenderEntities();
	static void RenderBorders();
	static void RenderScore();
	
	static bool IsPuckSpawnerFree();

	static void OnRestartClick();
	static void OnPlayerScore(Entity*, Entity*);
	static void OnPlayer1Score(Entity*, Entity*);
	static void OnPlayer2Score(Entity*, Entity*);
	static void OnStickCollision(Entity* entity1, Entity* entity2);
	static void OnPuckCollision(Entity* entity, Entity::mask_t layerMask);
};