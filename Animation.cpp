#include "Animation.h"

#include "Game.h"


FrameAnimation* FrameAnimation::CreateSingleFrame(SDL_Texture *texture)
{
	FrameAnimation* result = new FrameAnimation;

	SDL_Rect rect = {0, 0, 1, 1};

	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	result->AddFrame(texture, rect);

	return result;
}


FrameAnimation* FrameAnimation::CreateFromSpriteSheet2x2(SDL_Texture *texture, const std::vector<int>& frames)
{
	FrameAnimation* result = new FrameAnimation;

	std::vector<Frame> frameTemplates = CreateFramesFromSpriteSheet2x2(texture);

	for (int i : frames)
	{
		assert(static_cast<size_t>(i) < frameTemplates.size());
		result->m_frames.push_back(frameTemplates[i]);
	}

	return result;
}


const FrameAnimation::Frame& FrameAnimation::GetFrame(float moment) const
{
	assert(!m_frames.empty());

	if (m_duration > 0.0f)
	{
		float normalizedMoment = moment / m_duration;

		if (normalizedMoment > 1.0f) { normalizedMoment = 1.0f; }

		const int frameIndex = static_cast<int>((m_frames.size() - 1) * normalizedMoment);

		return m_frames[frameIndex];
	}
	else
	{
		return m_frames[0];
	}
}


void FrameAnimation::AddFrame(SDL_Texture* texture, SDL_Rect rect, int repetitions)
{
	SDL_assert(repetitions > 0);

	for (int i = 0; i < repetitions; i++)
	{
		m_frames.emplace_back(texture, rect);
	}
}


std::vector<FrameAnimation::Frame> FrameAnimation::CreateFramesFromSpriteSheet2x2(SDL_Texture *texture)
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

	result[0].rect.x = 0;
	result[0].rect.y = 0;

	result[1].rect.x = w;
	result[1].rect.y = 0;

	result[2].rect.x = 0;
	result[2].rect.y = h;

	result[3].rect.x = w;
	result[3].rect.y = h;

	return result;
}


SinusoidalTransparencyAnimation* SinusoidalTransparencyAnimation::Create(SDL_Texture *texture, float phase, float frequency)
{
	return Create(texture, phase, frequency, 0.0f, 1.0f);
}


SinusoidalTransparencyAnimation* SinusoidalTransparencyAnimation::Create(SDL_Texture *texture, float phase, float frequency, float minTransparency, float maxTransparency)
{
	SinusoidalTransparencyAnimation* result = new SinusoidalTransparencyAnimation;

	result->m_frame.texture = texture;
	result->m_frame.rect = { 0, 0, 1, 1 };
	result->m_phase = phase;
	result->m_frequency = frequency;
	result->m_minTransparency = fmaxf(minTransparency, 0.0f);
	result->m_maxTransparency = fminf(maxTransparency, 1.0f);

	return result;
}


const Animation::Frame& SinusoidalTransparencyAnimation::GetFrame(float moment) const
{
	const uint8_t aplha = static_cast<uint8_t>(255.0f * (1.0f - GetTransparency(moment)));

	SDL_SetTextureAlphaMod(m_frame.texture, aplha);

	return m_frame;
}


float SinusoidalTransparencyAnimation::GetTransparency(float moment) const
{
	const float s = sinf(m_phase + m_frequency * 2.0f * static_cast<float>(M_PI) * moment / m_duration) * 0.5f + 0.5f;
	return m_minTransparency + (m_maxTransparency - m_minTransparency) * s;
}