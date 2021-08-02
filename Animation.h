#pragma once

#include <vector>

#include <SDL.h>

#include <glm/glm.hpp>

#include "Event.h"


// all frames in animation has same duration, use multiple copies of frame
// to vary it's duration in comparison to others; this approach allows to use
// direct access to vector of frames through floating point moment in GetFrame() method


class AnimationController;


class Animation
{
	friend AnimationController;

public:
	struct Frame
	{
		SDL_Texture* texture;
		SDL_Rect rect;

		inline Frame() :
			texture(nullptr)
		{}

		inline Frame(SDL_Texture* texture, SDL_Rect rect) :
			texture(texture),
			rect(rect)
		{}

		Frame(const Frame& other) = default;
	};

public:
	Animation() = default;
	Animation(const Animation &other) = default;
	virtual ~Animation() = default;

	virtual const Frame& GetFrame(float moment) const = 0;

	inline float GetDuration() const { return m_duration; }

	inline void SetDuration(float duration) { m_duration = duration; }
	inline void SetLoop(bool enableLoop) { m_isLooped = enableLoop; }
	inline void SetNextState(const std::string& animation) { m_nextState = animation; }

	inline void OnFinish(std::function<void()> callback) { m_onFinish.AddListener(callback); }

protected:
	float m_duration;
	bool m_isLooped;
	Event<void()> m_onFinish;
	std::string m_nextState;
};


class FrameAnimation : public Animation
{
public:
	static FrameAnimation* CreateSingleFrame(SDL_Texture *texture);
	static FrameAnimation* CreateFromSpriteSheet2x2(SDL_Texture *texture, const std::vector<int>& frames);

	virtual const Frame& GetFrame(float moment) const override;

	void AddFrame(SDL_Texture* texture, SDL_Rect rect, int repetitions = 1);

protected:
	static std::vector<Frame> CreateFramesFromSpriteSheet2x2(SDL_Texture *texture);

protected:
	std::vector<Frame> m_frames;
};


class SinusoidalTransparencyAnimation : public Animation
{
public:
	static SinusoidalTransparencyAnimation* Create(SDL_Texture *texture, float phase, float frequency);
	static SinusoidalTransparencyAnimation* Create(SDL_Texture *texture, float phase, float frequency, float minTransparency, float maxTransparency);

	virtual const Frame& GetFrame(float moment) const override;

	float GetTransparency(float moment) const;

protected:
	Frame m_frame;

	float m_frequency;
	float m_phase;
	float m_minTransparency;
	float m_maxTransparency;
};