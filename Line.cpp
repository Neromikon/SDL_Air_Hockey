#include "Line.h"

#include "Circle.h"
#include "Rectangle.h"


line::line() :
	point1(0.0f, 0.0f),
	point2(1.0f, 1.0f)
{}


line::line(const glm::vec2& point1, const glm::vec2& point2) :
	point1(point1),
	point2(point2)
{}


bool line::Contact(const line& other) const
{
	const float k_errorCoefficient = 1.001f;

	const glm::vec2 left = glm::normalize(other.point1 - point1);
	const glm::vec2 right = glm::normalize(other.point2 - point1);
	const glm::vec2 middle = glm::normalize(point2 - point1);

	const float leftCosinus = glm::abs(glm::dot(left, middle));
	const float rightCosinus = glm::abs(glm::dot(right, middle));
	const float commonCosinus = glm::abs(glm::dot(left, right)) * k_errorCoefficient;

	return (leftCosinus <= commonCosinus) && (rightCosinus <= commonCosinus);
}


bool line::Contact(const circle& other) const
{
	const glm::vec2 distance1 = other.position - point1;
	if (glm::dot(distance1, distance1) <= other.radius * other.radius) { return true; }

	const glm::vec2 distance2 = other.position - point2;
	if (glm::dot(distance2, distance2) <= other.radius * other.radius) { return true; }

	const glm::vec2 lineDirection = glm::normalize(point2 - point1);
	const glm::vec2 projectionDirection = lineDirection * glm::dot(lineDirection, distance1) - (distance1);

	if (glm::length(projectionDirection) > other.radius) { return false; }

	const float dotprod1 = glm::dot(distance1, lineDirection);
	const float dotprod2 = glm::dot(distance2, -lineDirection);

	return (dotprod1 >= 0.0f) && (dotprod2 >= 0.0f);
}


bool line::Contact(const rectangle& other) const
{
	assert(false);
	return false; //TODO
}


glm::vec2 line::Nearest(const glm::vec2& from) const
{
	const glm::vec2 distance1 = from - point1;
	const glm::vec2 distance2 = from - point2;
	const glm::vec2 lineDirection = glm::normalize(point2 - point1);

	const float dotprod1 = glm::dot(distance1, lineDirection);
	const float dotprod2 = glm::dot(distance2, -lineDirection);

	if ((dotprod1 >= 0.0f) && (dotprod2 >= 0.0f))
	{
		const glm::vec2 projectionPoint = point1 + lineDirection * glm::dot(lineDirection, distance1);
		return projectionPoint;
	}
	else
	{
		if (glm::length(distance1) < glm::length(distance2))
		{
			return point1;
		}
		else
		{
			return point2;
		}
	}
}