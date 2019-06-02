#pragma once

#include <vector>

#include <SDL.h>

#include <glm/glm.hpp>

#include "Event.h"


// all frames in animation has same duration, use multiple copies of frame
// to vary it's duration in comparison to others; this approach allows to use
// direct access to vector of frames through floating point moment in GetFrame() method


class AnimationController;


class Animation final
{
	friend AnimationController;

public:
	struct Frame
	{
		SDL_Texture* texture;
		SDL_Rect rect;

		inline Frame():
			texture(nullptr)
		{}

		inline Frame(SDL_Texture* texture, SDL_Rect rect):
			texture(texture),
			rect(rect)
		{}

		Frame(const Frame& other) = default;
	};

	Animation() = default;
	Animation(const Animation &other) = default;
	~Animation() = default;

	static Animation CreateSingleFrame(SDL_Texture *texture);
	static Animation CreateFromSpriteSheet2x2(SDL_Texture *texture, const std::vector<int>& frames);

	Animation& operator=(const Animation& other);
	Animation& operator=(Animation&& other);

	const Frame& GetFrame(float moment);

	void AddFrame(SDL_Texture* texture, SDL_Rect rect, int repetitions = 1);

	inline float GetDuration() const { return m_duration; }

	inline void SetDuration(float duration) { m_duration = duration; }
	inline void SetLoop(bool enableLoop) { m_isLooped = enableLoop; }
	inline void SetNextState(const std::string& animation) { m_nextState = animation; }

	inline void OnFinish(std::function<void()> callback) { m_onFinish.AddListener(callback); }

protected:
	float m_duration;
	bool m_isLooped;
	std::vector<Frame> m_frames;
	Event<void()> m_onFinish;
	std::string m_nextState;

	static std::vector<Frame> CreateFramesFromSpriteSheet2x2(SDL_Texture *texture);
};