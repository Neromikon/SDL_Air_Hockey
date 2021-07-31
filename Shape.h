#pragma once

#include "Line.h"
#include "Circle.h"
#include "Rectangle.h"


struct shape final
{
	enum Type : unsigned char { LINE, CIRCLE, RECTANGLE };

	union Data
	{
		line m_line;
		circle m_circle;
		rectangle m_rectangle;

		inline Data() {}
	};

	Type m_type;
	Data m_data;

	shape();

	float Contact(const shape& other) const;

	template <typename type> float Contact(const type& other) const;

	void Translate(const glm::vec2& distance);

private:
	inline static constexpr unsigned char Combination(Type type1, Type type2) { return static_cast<unsigned char>((type1 << 4) | type2); }
};


#include "Shape.inl"