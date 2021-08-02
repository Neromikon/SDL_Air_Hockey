#include <chrono>

#include "Controller.h"

#include "Game.h"
#include "Entity.h"


namespace
{
	const float k_maxSpeed = 1.15f;

	const float k_enoughDestinationDistance = 0.008f;
	const float k_defendDistance = 0.15f;
	const float k_minimumDirectionCosinus = 0.5f;

	const float k_minAiNextRoundWaitDuration = 0.1f; // seconds; gives player a chance to hit the puck firts
	const float k_maxAiNextRoundWaitDuration = 2.2f;

	const float k_guardPointChangePeriod = 6.0f; // seconds
	const float k_reverseGuardPointChangePeriod = 1.0f / k_guardPointChangePeriod; // seconds
}


KeyboardController::KeyboardController() :
	m_down(SDL_Scancode::SDL_SCANCODE_DOWN),
	m_left(SDL_Scancode::SDL_SCANCODE_LEFT),
	m_right(SDL_Scancode::SDL_SCANCODE_RIGHT),
	m_up(SDL_Scancode::SDL_SCANCODE_UP),
	m_moveDirection(0.0f, 0.0f),
	m_moveDown(false),
	m_moveLeft(false),
	m_moveRight(false),
	m_moveUp(false)
{}


void KeyboardController::Update()
{
	m_moveDirection = GetDirection();

	if (!m_ownArea.Contain(m_controlTarget->GetPosition()))
	{
		const glm::vec2 nearestToArea = m_ownArea.Nearest(m_controlTarget->GetPosition());
		const glm::vec2 allowedDirection = glm::normalize(nearestToArea - m_controlTarget->GetPosition());
		const glm::vec2 tangent(allowedDirection.y, -allowedDirection.x);

		//const float normalSpeed = glm::max(glm::dot(m_moveDirection, allowedDirection), 0.0f);

		m_moveDirection = glm::normalize(allowedDirection + tangent * glm::dot(m_moveDirection, tangent));
	}

	m_controlTarget->AccelerateWithLimit(m_moveDirection * m_moveForce, k_maxSpeed);
}


void KeyboardController::OnKeyboardDown(SDL_Scancode sdlScancode)
{
	if (m_down == sdlScancode) { m_moveDown = true; } else
	if (m_left == sdlScancode) { m_moveLeft = true; } else
	if (m_right == sdlScancode) { m_moveRight = true; } else
	if (m_up == sdlScancode) { m_moveUp = true; }
}


void KeyboardController::OnKeyboardUp(SDL_Scancode sdlScancode)
{
	if (m_down == sdlScancode) { m_moveDown = false; } else
	if (m_left == sdlScancode) { m_moveLeft = false; } else
	if (m_right == sdlScancode) { m_moveRight = false; } else
	if (m_up == sdlScancode) { m_moveUp = false; }
}


glm::vec2 KeyboardController::GetDirection() const
{
	const float SQRT2 = 0.70710678118f;

	const unsigned char directionCode = m_moveDown | (m_moveLeft << 1) | (m_moveRight << 2) | (m_moveUp << 3);

	switch (directionCode)
	{
		case 0b0001: return glm::vec2(0.0f, -1.0f);
		case 0b0010: return glm::vec2(-1.0f, 0.0f);
		case 0b0100: return glm::vec2(1.0f, 0.0f);
		case 0b1000: return glm::vec2(0.0f, 1.0f);
		case 0b0011: return glm::vec2(-SQRT2, -SQRT2);
		case 0b0101: return glm::vec2(SQRT2, -SQRT2);
		case 0b1010: return glm::vec2(-SQRT2, SQRT2);
		case 0b1100: return glm::vec2(SQRT2, SQRT2);
		case 0b0111: return glm::vec2(0.0f, -1.0f);
		case 0b1011: return glm::vec2(-1.0f, 0.0f);
		case 0b1101: return glm::vec2(1.0f, 0.0f);
		case 0b1110: return glm::vec2(0.0f, 1.0f);
		default: return glm::vec2(0.0f, 0.0f);
	}
}


AIController::AIController()
{
	srand(std::chrono::steady_clock::now().time_since_epoch().count());
	m_gateGuardPointPhase = 0.001f * (rand() % 1000);

	Game::s_onNextRound.AddListener([this]() { OnNextRound(); });
	Game::s_onPuckCollision.AddListener([this]() { OnPuckCollision(); });
}


void AIController::Update()
{
	glm::vec2 moveDirection(0.0f, 0.0f);

	m_remainingWaiting = fmaxf(m_remainingWaiting - Game::deltaTime, 0.0f);

	if (m_remainingWaiting <= 0.0f && m_puck->IsEnabled() && m_ownArea.Contain(m_puck->GetPosition()))
	{
		if (!m_puck->IsMoving())
		{
			const glm::vec2 directionToPuck = glm::normalize(m_puck->GetPosition() - m_controlTarget->GetPosition());
			moveDirection = directionToPuck;
		}
		else
		{
			const glm::vec2 referencePoint = m_opponentGateRectangle.Nearest(m_puck->GetPosition());

			const glm::vec2 directionToPuck = glm::normalize(m_puck->GetPosition() - m_controlTarget->GetPosition());
			const glm::vec2 directionToOpponentGate = glm::normalize(referencePoint - m_puck->GetPosition());

			const float cosinus = glm::dot(directionToPuck, directionToOpponentGate);

			if (cosinus > k_minimumDirectionCosinus)
			{
				if (m_controlTarget->GetPosition() != m_puck->GetPosition())
				{
					moveDirection = directionToPuck;
				}
			}
		}
	}
	
	if(moveDirection.x == 0.0f && moveDirection.y == 0.0f)
	{
		m_gateGuardPointPhase += Game::deltaTime * k_reverseGuardPointChangePeriod;
		m_gateGuardPointPhase -= static_cast<float>(static_cast<int>(m_gateGuardPointPhase));

		const float gateWidth = glm::length(m_ownGateRectangle.axis1);
		const glm::vec2 phaseOffset(cosf(m_gateGuardPointPhase * 2.0f * M_PI) * gateWidth * 0.25f, 0.0f);
		const glm::vec2 referencePoint = m_ownGateRectangle.Nearest(m_puck->GetPosition()) + phaseOffset;
		const glm::vec2 aimPoint = referencePoint + glm::normalize(m_puck->GetPosition() - referencePoint) * k_defendDistance;

		if (glm::distance(m_controlTarget->GetPosition(), aimPoint) > k_enoughDestinationDistance)
		{
			moveDirection = glm::normalize(aimPoint - m_controlTarget->GetPosition());
		}
	}

	m_controlTarget->AccelerateWithLimit(moveDirection * m_moveForce, k_maxSpeed);
}

void AIController::OnNextRound()
{
	srand(std::chrono::steady_clock::now().time_since_epoch().count());
	
	const float randomValue = 0.0001f * static_cast<float>(rand() % 10000);
	const float randomTime = (k_maxAiNextRoundWaitDuration - k_minAiNextRoundWaitDuration) * randomValue;

	m_remainingWaiting = k_minAiNextRoundWaitDuration + randomTime;
}

void AIController::OnPuckCollision()
{
	m_remainingWaiting = 0.0f;
}