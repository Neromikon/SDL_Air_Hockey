#include "Rectangle.h"

#include "Line.h"
#include "Circle.h"

#include <glm/gtx/norm.hpp>


rectangle::rectangle() :
	position(0.0f, 0.0f),
	axis1(1.0f, 0.0f),
	axis2(0.0f, 1.0f)
{}


rectangle::rectangle(const glm::vec2 &position, const glm::vec2 &size) :
	position(position),
	axis1(size.x * 0.5f, 0.0f),
	axis2(0.0f * 0.5f, size.y)
{}


rectangle::rectangle(const glm::vec2 &position, const glm::vec2 &axis1, const glm::vec2 &axis2) :
	position(position),
	axis1(axis1),
	axis2(axis2)
{}


bool rectangle::Contain(const glm::vec2 &point) const
{
	if (glm::abs(glm::dot(glm::normalize(axis1), point - position)) > glm::length(axis1)) { return false; }
	if (glm::abs(glm::dot(glm::normalize(axis2), point - position)) > glm::length(axis2)) { return false; }

	return true;
}


float rectangle::Contact(const line& other) const
{
	return other.Contact(*this);
}


float rectangle::Contact(const circle &other) const
{
	const glm::vec2 distance = other.position - position;
	const glm::vec2 normalizedAxis1 = glm::normalize(axis1);
	const glm::vec2 normalizedAxis2 = glm::normalize(axis2);
	const float distanceByAxis1 = glm::abs(glm::dot(normalizedAxis1, distance));
	const float distanceByAxis2 = glm::abs(glm::dot(normalizedAxis2, distance));

	if (distanceByAxis1 > glm::length(axis1) + other.radius) { return false; }
	if (distanceByAxis2 > glm::length(axis2) + other.radius) { return false; }

	return 1.0f;
}


float rectangle::Contact(const rectangle &other) const
{
	const glm::vec2 size(axis1.length(), axis2.length());
	const glm::vec2 otherSize(other.axis1.length(), other.axis2.length());

	if (position.x - size.x > other.position.x + otherSize.x) { return 0.0f; }
	if (position.y - size.y > other.position.y + otherSize.y) { return 0.0f; }
	if (position.x + size.x < other.position.x - otherSize.x) { return 0.0f; }
	if (position.y + size.y < other.position.y - otherSize.y) { return 0.0f; }

	return 1.0f;
}


float rectangle::Radius() const
{
	return glm::length((axis1 + axis2));
}


circle rectangle::BoundingCircle() const
{
	return circle(position, glm::length(axis1 + axis2));
}


glm::vec2 rectangle::Nearest(const glm::vec2& from) const
{
	const glm::vec2 corner1 = position + axis1 + axis2;
	const glm::vec2 corner2 = position + axis1 - axis2;
	const glm::vec2 corner3 = position - axis1 + axis2;
	const glm::vec2 corner4 = position - axis1 - axis2;

	//check if against axis1
	{
		const glm::vec2 edge = glm::normalize(corner2 - corner1);
		const glm::vec2 distance1 = from - corner1;
		const glm::vec2 distance2 = from - corner2;
		const float cosinus1 = glm::dot(distance1, edge);
		const float cosinus2 = glm::dot(distance2, -edge);

		if (cosinus1 >= 0 && cosinus2 >= 0)
		{
			if (glm::dot(from - position, axis1) > 0)
			{
				return corner1 + edge * glm::dot(edge, from - corner1);
			}
			else
			{
				return corner3 + edge * glm::dot(edge, from - corner3);
			}
		}
	}

	//check if against axis2
	{
		const glm::vec2 edge = glm::normalize(corner3 - corner1);
		const glm::vec2 distance1 = from - corner1;
		const glm::vec2 distance2 = from - corner3;
		const float cosinus1 = glm::dot(distance1, edge);
		const float cosinus2 = glm::dot(distance2, -edge);

		if (cosinus1 >= 0 && cosinus2 >= 0)
		{
			if (glm::dot(from - position, axis2) > 0)
			{
				return corner1 + edge * glm::dot(edge, from - corner1);
			}
			else
			{
				return corner2 + edge * glm::dot(edge, from - corner2);
			}
		}
	}

	//pick nearest corner
	{
		const float sqrDistance1 = glm::distance2(from, corner1);
		const float sqrDistance2 = glm::distance2(from, corner2);
		const float sqrDistance3 = glm::distance2(from, corner3);
		const float sqrDistance4 = glm::distance2(from, corner4);

		const float minDistance = glm::min(glm::min(sqrDistance1, sqrDistance2), glm::min(sqrDistance3, sqrDistance4));

		if (sqrDistance1 == minDistance) { return corner1; }
		if (sqrDistance2 == minDistance) { return corner2; }
		if (sqrDistance3 == minDistance) { return corner3; }

		return corner4;
	}
}


void rectangle::Resize(const glm::vec2& size)
{
	axis1 = glm::normalize(axis1) * size.x;
	axis2 = glm::normalize(axis2) * size.y;
}