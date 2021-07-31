#pragma once

#include <glm/glm.hpp>

#include <SDL.h>

#include "Entity.h" //temp, replace with Rectangle.h


class Entity;


class Controller
{
public:
	Controller() :
		m_controlTarget(nullptr),
		m_moveForce(1.0f)
	{}

	virtual ~Controller() = default;

	virtual void Update() = 0;

	void SetControlTarget(Entity *entity)
	{
		m_controlTarget = entity;
	}

	inline void SetMoveForce(float force) { m_moveForce = force; }
	inline void SetArea(const rectangle& area) { m_ownArea = area; }

protected:
	Entity *m_controlTarget;

	float m_moveForce;

	rectangle m_ownArea;
};


class KeyboardController final : public Controller
{
public:
	KeyboardController();

	void Update() override;

	void OnKeyboardDown(SDL_Scancode sdlScancode);
	void OnKeyboardUp(SDL_Scancode sdlScancode);

private:
	SDL_Scancode m_down, m_left, m_right, m_up;
	glm::vec2 m_moveDirection;
	bool m_moveDown, m_moveLeft, m_moveRight, m_moveUp;

	glm::vec2 GetDirection() const;
};


class AIController final : public Controller
{
public:
	AIController();

	void Update() override;

	inline void SetPuck(Entity *puck) { m_puck = puck; }
	inline void SetOwnGateRectangle(const rectangle& gate) { m_ownGateRectangle = gate; }
	inline void SetOpponentGateRectangle(const rectangle& gate) { m_opponentGateRectangle = gate; }

private:
	void OnNextRound();
	void OnPuckCollision();
	
private:
	Entity *m_puck;
	rectangle m_ownGateRectangle;
	rectangle m_opponentGateRectangle;
	float m_remainingWaiting;
};