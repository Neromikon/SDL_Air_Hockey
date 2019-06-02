#pragma once


template<typename ShapeType>
bool Entity::Contact(const ShapeType &other, mask_t collisionMask) const
{
	if (!(m_layerMask & collisionMask)) { return false; }

	const float k_timeStep = 0.025f;

	if (IsMoving())
	{
		shape tempShape = m_shape;

		for (float i = 0.0f; i < 1.0f; i += k_timeStep)
		{
			if (tempShape.Contact(other)) { return true; }

			tempShape.Translate(m_velocity * Game::deltaTime * k_timeStep);
		}

		return false;
	}
	else
	{
		return m_shape.Contact(other);
	}
}