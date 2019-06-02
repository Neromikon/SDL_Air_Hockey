#pragma once

#include <glm/glm.hpp>


struct line;
struct circle;
struct rectangle;


struct line final
{
	glm::vec2 point1, point2;

	line();
	line(const glm::vec2& point1, const glm::vec2& point2);

	bool Contact(const line& other) const;
	bool Contact(const circle& other) const;
	bool Contact(const rectangle& other) const;

	inline float length() const { return glm::length(point1 - point2); }

	glm::vec2 Nearest(const glm::vec2& from) const;
};