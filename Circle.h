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

	bool Contact(const line &other) const;
	bool Contact(const circle &other) const;
	bool Contact(const rectangle &other) const;
};