#pragma once


template<typename ShapeType>
float Entity::Contact(const ShapeType &other, mask_t collisionMask) const
{
	if (!(m_layerMask & collisionMask)) { return false; }

	const float k_timeStep = 0.025f;

	if (IsMoving())
	{
		shape tempShape = m_shape;

		for (float i = 0.0f; i < 1.0f; i += k_timeStep)
		{
			const float penetration = tempShape.Contact(other);
			if (penetration > 0.0f) { return penetration; }

			tempShape.Translate(m_velocity * Game::deltaTime * k_timeStep);
		}

		return 0.0f;
	}
	else
	{
		return m_shape.Contact(other);
	}
}