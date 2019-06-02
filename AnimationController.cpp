#include "AnimationController.h"

#include <glm/glm.hpp>

#include "Game.h"


AnimationController::AnimationController() :
	m_moment(0.0f),
	m_isPaused(false),
	m_currentAnimation("")
{}


void AnimationController::Update()
{
	if (m_isPaused) { return; }

	if (m_currentAnimation.empty())
	{
		if (m_animations.empty()) { return; }

		m_currentAnimation = m_animations.begin()->first;
	}

	const Animation& animation = m_animations[m_currentAnimation];

	if (animation.m_duration > 0.0f)
	{
		m_moment += Game::deltaTime;

		if (m_moment >= animation.m_duration)
		{
			animation.m_onFinish.Invoke();

			if (animation.m_isLooped)
			{
				m_moment -= animation.m_duration * glm::floor(m_moment / animation.m_duration);
			}
			else if (!animation.m_nextState.empty())
			{
				m_currentAnimation = animation.m_nextState;
			}
		}
	}
}


void AnimationController::Play(const std::string& animation)
{
	auto find = m_animations.find(animation);

	if (find == m_animations.end()) { return; }

	m_currentAnimation = animation;
	m_moment = 0.0f;
}


void AnimationController::PlayIfNotPlaying(const std::string& animation)
{
	if (m_currentAnimation != animation) { Play(animation); }
}