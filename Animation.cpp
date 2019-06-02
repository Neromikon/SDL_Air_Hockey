#include "Animation.h"

#include "Game.h"


Animation Animation::CreateSingleFrame(SDL_Texture *texture)
{
	Animation result;

	SDL_Rect rect = {0.0f, 0.0f, 1.0f, 1.0f};

	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	result.AddFrame(texture, rect);

	return result;
}


Animation Animation::CreateFromSpriteSheet2x2(SDL_Texture *texture, const std::vector<int>& frames)
{
	Animation result;

	std::vector<Frame> frameTemplates = CreateFramesFromSpriteSheet2x2(texture);

	for (int i : frames)
	{
		assert(i < frameTemplates.size());
		result.m_frames.push_back(frameTemplates[i]);
	}

	return result;
}


Animation& Animation::operator=(const Animation& other)
{
	m_duration = other.m_duration;
	m_isLooped = other.m_isLooped;
	m_onFinish = other.m_onFinish;
	m_nextState = other.m_nextState;
	m_frames = other.m_frames;

	return *this;
}


Animation& Animation::operator=(Animation&& other)
{
	m_duration = other.m_duration;
	m_isLooped = other.m_isLooped;
	m_onFinish = other.m_onFinish;
	m_nextState = other.m_nextState;

	m_frames = std::move(other.m_frames);

	return *this;
}


const Animation::Frame& Animation::GetFrame(float moment)
{
	assert(!m_frames.empty());

	if (m_duration > 0.0f)
	{
		float normalizedMoment = moment / m_duration;

		if (normalizedMoment > 1.0f) { normalizedMoment = 1.0f; }

		const int frameIndex = static_cast<float>(m_frames.size() - 1) * normalizedMoment;

		return m_frames[frameIndex];
	}
	else
	{
		return m_frames[0];
	}
}


void Animation::AddFrame(SDL_Texture* texture, SDL_Rect rect, int repetitions)
{
	SDL_assert(repetitions > 0);

	for (int i = 0; i < repetitions; i++)
	{
		m_frames.emplace_back(texture, rect);
	}
}


std::vector<Animation::Frame> Animation::CreateFramesFromSpriteSheet2x2(SDL_Texture *texture)
{
	std::vector<Frame> result(4);

	int w, h;

	SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);

	w /= 2;
	h /= 2;

	for (Frame &frame : result)
	{
		frame.texture = texture;
		frame.rect.w = w;
		frame.rect.h = h;
	}

	result[0].rect.x = 0.0f;
	result[0].rect.y = 0.0f;

	result[1].rect.x = w;
	result[1].rect.y = 0.0f;

	result[2].rect.x = 0.0f;
	result[2].rect.y = h;

	result[3].rect.x = w;
	result[3].rect.y = h;

	return result;
}