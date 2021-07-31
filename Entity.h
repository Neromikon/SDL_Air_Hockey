#pragma once

#include <iostream>

#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Event.h"
#include "Shape.h"
#include "AnimationController.h"


class Entity
{
public:
	using mask_t = unsigned int;

	static const unsigned int STICK_LAYER = 1 << 0;
	static const unsigned int PUCK_LAYER = 1 << 1;
	static const unsigned int WALL_LAYER = 1 << 2;
	static const unsigned int GATE_LAYER = 1 << 3;

	Entity();
	virtual ~Entity() = default;

	float Contact(const Entity &other) const;

	template<typename ShapeType> float Contact(const ShapeType &other, mask_t collisionMask = 0xFFFFFFFF) const;

	void Collide(Entity &other, float penetration);
	void ReflectFrom(const line& other, mask_t layerMask, float consumedVelocityRatio = 0.0f);

	virtual void Update();

	void Draw();

	void AccelerateWithLimit(const glm::vec2& acceleration, float maxSpeed);

	bool IsEnabled() const { return m_isEnabled; }
	bool IsStatic() const { return m_isStatic; }
	bool IsPhysical() const { return m_isPhysical; }
	const glm::vec2& GetPosition() const { return m_position; }
	const shape& GetShape() const { return m_shape; }

	void SetPosition(const glm::vec2& position);
	void SetAnchoredPosition(const glm::vec2& position, const glm::vec2 &anchor);
	void SetDoubleAnchoredPosition(const glm::vec2 &anchor1, const glm::vec2 &anchor2);
	void SetSize(const glm::vec2& size);
	void SetShape(shape::Type type);

	inline bool IsMoving() const { return (m_velocity.x != 0) || (m_velocity.y != 0); }
	inline bool CanCollideWith(unsigned int layerMask) { return m_collisionMask & layerMask; }

	inline void Play(const std::string& animation) { m_animationController.Play(animation); }
	inline void PlayIfNotPlaying(const std::string& animation) { m_animationController.PlayIfNotPlaying(animation); }
	inline void Pause() { m_animationController.Pause(); }
	inline void Resume() { m_animationController.Resume(); }

	inline Animation& AddAnimation(const std::string& name, const Animation &animation) { return m_animationController.AddAnimation(name, animation); }
	inline Animation& GetAnimation(const std::string& animation) { return m_animationController.GetAnimation(animation); }

	inline void SetEnabled(bool enabled) { m_isEnabled = enabled; }
	inline void SetName(const std::string &name) { m_name = name; }
	inline void SetRenderer(SDL_Renderer *renderer) { m_sdlRenderer = renderer; }
	inline void SetVelocity(const glm::vec2& velocity) { m_velocity = velocity; }
	inline void SetMass(float mass) { m_mass = mass; }
	inline void SetStatic(bool isStatic) { m_isStatic = isStatic; }
	inline void SetLayerMask(unsigned int mask) { m_layerMask = mask; }
	inline void SetCollisionMask(unsigned int mask) { m_collisionMask = mask; }
	inline void SetFrinction(float friction) { m_friction = friction; }

public:
	Event<void(Entity*, Entity*)> m_onCollision;
	Event<void(Entity*, mask_t layerMask)> m_onCollisionWithLayer;

private:
	bool m_isEnabled;
	bool m_isStatic;
	bool m_isPhysical;

	std::string m_name;

	glm::vec2 m_position;
	glm::quat m_orientation;
	glm::vec2 m_size;

	shape m_shape;
	float m_mass;
	glm::vec2 m_velocity;
	float m_friction;
	mask_t m_layerMask; //to which layers that object belongs
	mask_t m_collisionMask; //with which layers this object can collide

	SDL_Renderer *m_sdlRenderer;
	SDL_Rect m_sdlRect;

	AnimationController m_animationController;

	void UpdateRect();
	void UpdateShape();
	void ApplyFriction();
};


#include "Entity.inl"