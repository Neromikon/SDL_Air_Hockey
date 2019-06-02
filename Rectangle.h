#pragma once

#include <glm/glm.hpp>


struct line;
struct circle;
struct rectangle;


struct rectangle final
{
	glm::vec2 position;
	glm::vec2 axis1, axis2;

	rectangle();
	rectangle(const glm::vec2& position, const glm::vec2& size);
	rectangle(const glm::vec2& position, const glm::vec2& axis1, const glm::vec2& axis2);

	bool Contain(const glm::vec2& point) const;

	bool Contact(const line& other) const;
	bool Contact(const circle& other) const;
	bool Contact(const rectangle& other) const;

	float Radius() const;
	circle BoundingCircle() const;
	glm::vec2 Nearest(const glm::vec2& from) const;

	void Resize(const glm::vec2& size);
};