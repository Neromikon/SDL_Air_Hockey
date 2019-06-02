#include "Circle.h"

#include "Line.h"
#include "Rectangle.h"


circle::circle():
	position(0.0f, 0.0f),
	radius(1.0f)
{}


circle::circle(const glm::vec2 &center, float radius):
	position(center),
	radius(radius)
{}


bool circle::Contact(const line &other) const
{
	return other.Contact(*this);
}


bool circle::Contact(const circle &other) const
{
	return glm::distance(position, other.position) <= radius + other.radius;
}


bool circle::Contact(const rectangle &other) const
{
	return other.Contact(*this);
}