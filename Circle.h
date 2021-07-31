#pragma once

#include <glm/glm.hpp>


struct line;
struct circle;
struct rectangle;


struct circle final
{
	glm::vec2 position;
	float radius;

	circle();
	circle(const glm::vec2 &center, float radius);

	float Contact(const line &other) const;
	float Contact(const circle &other) const;
	float Contact(const rectangle &other) const;
};