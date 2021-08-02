#pragma once

#include <map>
#include <string>

#include "Animation.h"


class AnimationController
{
public:
	AnimationController();
	~AnimationController();

	void Update();

	void Play(const std::string& animation);
	void PlayIfNotPlaying(const std::string& animation);

	inline const std::string& GetCurrentAnimation() { return m_currentAnimation; }
	inline const Animation::Frame& GetCurrentFrame() { return m_animations[m_currentAnimation]->GetFrame(m_moment); }
	inline Animation* GetAnimation(const std::string& animation) { return m_animations[animation]; }
	inline bool HaveAnimation(const std::string& animation) { return m_animations.find(animation) != m_animations.end(); }

	inline void Pause() { m_isPaused = true; }
	inline void Resume() { m_isPaused = false; }
	inline void Restart() { m_moment = 0.0f; }
	inline void SetMoment(float moment) { m_moment = moment; }

	inline Animation* AddAnimation(const std::string& name, Animation* animation) { return (m_animations[name] = animation); }

private:
	float m_moment;
	bool m_isPaused;

	std::map<std::string, Animation*> m_animations;
	std::string m_currentAnimation;
};