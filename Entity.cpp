#include "Entity.h"

#include "Game.h"


namespace
{
	const float kPenetrationRepellingCoefficient = 1.5f;
}


shape::shape() :
	m_type(CIRCLE)
{}


float shape::Contact(const shape& other) const
{
	switch (shape::Combination(m_type, other.m_type))
	{
		case shape::Combination(shape::LINE, shape::LINE): return m_data.m_line.Contact(other.m_data.m_line);
		case shape::Combination(shape::LINE, shape::CIRCLE): return m_data.m_line.Contact(other.m_data.m_circle);
		case shape::Combination(shape::LINE, shape::RECTANGLE): return m_data.m_line.Contact(other.m_data.m_rectangle);
		case shape::Combination(shape::CIRCLE, shape::LINE): return m_data.m_circle.Contact(other.m_data.m_line);
		case shape::Combination(shape::CIRCLE, shape::CIRCLE): return m_data.m_circle.Contact(other.m_data.m_circle);
		case shape::Combination(shape::CIRCLE, shape::RECTANGLE): return m_data.m_circle.Contact(other.m_data.m_rectangle);
		case shape::Combination(shape::RECTANGLE, shape::LINE): return m_data.m_rectangle.Contact(other.m_data.m_line);
		case shape::Combination(shape::RECTANGLE, shape::CIRCLE): return m_data.m_rectangle.Contact(other.m_data.m_circle);
		case shape::Combination(shape::RECTANGLE, shape::RECTANGLE): return m_data.m_rectangle.Contact(other.m_data.m_rectangle);

		default:
		{
			SDL_assert(false);
			std::cerr << "Unhandled combination of geomentry shapes\n";
			return 0.0f;
		}
	}
}


void shape::Translate(const glm::vec2& distance)
{
	switch (m_type)
	{
		case Type::LINE:
		{
			m_data.m_line.point1 += distance;
			m_data.m_line.point2 += distance;
			return;
		}

		case Type::CIRCLE: m_data.m_circle.position += distance; return;
		case Type::RECTANGLE: m_data.m_rectangle.position += distance; return;
	}
}


Entity::Entity() :
	m_isEnabled(true),
	m_isStatic(false),
	m_isPhysical(true),
	m_position(0.0f, 0.0f),
	m_size(1.0f, 1.0f),
	m_mass(0.0f),
	m_velocity(0.0f, 0.0f),
	m_sdlRenderer(nullptr)
{
	UpdateRect();
}


float Entity::Contact(const Entity &other) const
{
	if (!(m_layerMask & other.m_collisionMask)) { return false; }

	const float k_timeStep = 0.05f;

	if (IsMoving() || other.IsMoving())
	{
		shape tempShape1 = m_shape;
		shape tempShape2 = other.m_shape;

		for (float i = 0.0f; i < 1.0f; i += k_timeStep)
		{
			const float penetration = tempShape1.Contact(tempShape2);
			if (penetration > 0.0f) { return penetration; }

			tempShape1.Translate(m_velocity * Game::deltaTime * k_timeStep);
			tempShape2.Translate(other.m_velocity * Game::deltaTime * k_timeStep);
		}
	}
	else
	{
		return m_shape.Contact(other.m_shape);
	}

	return 0.0f;
}


void Entity::Collide(Entity &other, float penetration)
{
	m_onCollision.Invoke(this, &other);
	other.m_onCollision.Invoke(&other, this);

	m_onCollisionWithLayer.Invoke(this, other.m_layerMask);
	other.m_onCollisionWithLayer.Invoke(&other, m_layerMask);

	if (!m_isPhysical || !other.m_isPhysical) { return; }

	if (!m_isStatic && !other.m_isStatic)
	{
		const glm::vec2 normal = glm::normalize(other.m_position - m_position);
		const glm::vec2 tangent = glm::vec2(normal.y, -normal.x); //rotate 90 degrees

		const float normalSpeed1 = glm::dot(m_velocity, normal);
		const float tangentSpeed1 = glm::dot(m_velocity, tangent);

		const float normalSpeed2 = glm::dot(other.m_velocity, normal);
		const float tangentSpeed2 = glm::dot(other.m_velocity, tangent);

		const float reverseMassSum = 1.0f / (m_mass + other.m_mass);

		const float newNormalSpeed1 = ((m_mass - other.m_mass) * normalSpeed1 + 2.0f * other.m_mass * normalSpeed2) * reverseMassSum;
		const float newNormalSpeed2 = ((other.m_mass - m_mass) * normalSpeed2 + 2.0f * m_mass * normalSpeed1) * reverseMassSum;

		const float repellingSpeed = -penetration * kPenetrationRepellingCoefficient * 0.5f;

		m_velocity = (newNormalSpeed1 + repellingSpeed) * normal + tangentSpeed1 * tangent;
		other.m_velocity = (newNormalSpeed2 - repellingSpeed) * normal + tangentSpeed2 * tangent;
	}
	else
	{
		if (m_isStatic && other.m_isStatic) { return; }

		if (m_isStatic && !other.m_isStatic)
		{
			const glm::vec2 normal = glm::normalize(other.m_position - m_position);
			const glm::vec2 tangent = glm::vec2(normal.y, -normal.x); //rotate 90 degrees

			const float normalSpeed2 = glm::dot(other.m_velocity, normal);
			const float tangentSpeed2 = glm::dot(other.m_velocity, tangent);

			other.m_velocity = -normalSpeed2 * normal + tangentSpeed2 * tangent;
		}
		else
		{
			const glm::vec2 normal = glm::normalize(other.m_position - m_position);
			const glm::vec2 tangent = glm::vec2(normal.y, -normal.x); //rotate 90 degrees

			const float normalSpeed1 = glm::dot(m_velocity, normal);
			const float tangentSpeed1 = glm::dot(m_velocity, tangent);

			m_velocity = -normalSpeed1 * normal + tangentSpeed1 * tangent;
		}
	}
}


void Entity::ReflectFrom(const line& other, mask_t layerMask, float consumedVelocityRatio)
{
	SDL_assert(consumedVelocityRatio >= 0.0f && consumedVelocityRatio <= 1.0f);

	m_onCollisionWithLayer.Invoke(this, layerMask);

	const glm::vec2 nearestPoint = other.Nearest(m_position);

	const glm::vec2 normal = glm::normalize(nearestPoint - m_position);
	const glm::vec2 tangent(normal.y, -normal.x); //rotate 90 degrees

	const float normalSpeed = glm::dot(m_velocity, normal) * (1.0f - consumedVelocityRatio);
	const float tangentSpeed = glm::dot(m_velocity, tangent);

	m_velocity = -normalSpeed * normal + tangentSpeed * tangent;	
}


void Entity::Update()
{
	m_animationController.Update();

	if (IsMoving())
	{
		m_position += m_velocity * Game::deltaTime;
		UpdateRect();
		UpdateShape();

		if (m_friction > 0) { ApplyFriction(); }
	}
}

void Entity::Draw()
{
	if (!m_sdlRenderer) { return; }

	if (m_animationController.GetCurrentAnimation().empty()) { return; }

	const Animation::Frame& frame = m_animationController.GetCurrentFrame();

	SDL_RenderCopy(m_sdlRenderer, frame.texture, &frame.rect, &m_sdlRect);
}


void Entity::AccelerateWithLimit(const glm::vec2& acceleration, float maxSpeed)
{
	if (glm::length(m_velocity) < maxSpeed)
	{
		m_velocity += acceleration * Game::deltaTime;
	}
}


void Entity::SetPosition(const glm::vec2& position)
{
	m_position = position;
	UpdateRect();
	UpdateShape();
}


void Entity::SetAnchoredPosition(const glm::vec2& position, const glm::vec2& anchor)
{
	m_position.x = anchor.x + position.x;
	m_position.y = anchor.y * Game::reverseWindowRatio + position.y;
	UpdateRect();
	UpdateShape();
}


void Entity::SetDoubleAnchoredPosition(const glm::vec2 &anchor1, const glm::vec2 &anchor2)
{
	m_position.x = 0.5f * (anchor1.x + anchor2.x);
	m_position.y = 0.5f * (anchor1.y + anchor2.y) * Game::reverseWindowRatio;
	SetSize(glm::abs(anchor1 - anchor2) * glm::vec2(1.0f, Game::reverseWindowRatio));
	UpdateRect();
	UpdateShape();
}


void Entity::SetSize(const glm::vec2& size)
{
	m_size = size;
	UpdateRect();
	UpdateShape();
}


void Entity::SetShape(shape::Type type)
{
	m_shape.m_type = type;
	UpdateShape();
}


void Entity::UpdateRect()
{
	const float x = Game::windowSize.x * m_position.x;
	const float y = Game::windowSize.x * m_position.y;
	m_sdlRect.x = static_cast<int>(x) - m_sdlRect.w / 2;
	m_sdlRect.y = Game::windowSize.y - static_cast<int>(y) - m_sdlRect.h / 2;
	m_sdlRect.w = m_size.x * Game::windowSize.x;
	m_sdlRect.h = m_size.y * Game::windowSize.x;
}


void Entity::UpdateShape()
{
	switch (m_shape.m_type)
	{
		case shape::CIRCLE:
		{
			m_shape.m_data.m_circle.position = m_position;
			m_shape.m_data.m_circle.radius = glm::max(m_size.x, m_size.y) * 0.5f;
		}

		case shape::RECTANGLE:
		{
			m_shape.m_data.m_rectangle.position = m_position;
			m_shape.m_data.m_rectangle.axis1 = glm::vec2(m_size.x * 0.5f, 0.0f);
			m_shape.m_data.m_rectangle.axis2 = glm::vec2(0.0f, m_size.y * 0.5f);
		}
	}
}


void Entity::ApplyFriction()
{
	float speed = glm::length(m_velocity);

	speed -= m_friction * Game::deltaTime;

	if (speed < 0) { speed = 0; }

	m_velocity = glm::normalize(m_velocity) * speed;
}