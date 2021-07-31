#pragma once


template <typename type>
float shape::Contact(const type& other) const
{
	switch (m_type)
	{
		case Type::LINE: return m_data.m_line.Contact(other);
		case Type::CIRCLE: return m_data.m_circle.Contact(other);
		case Type::RECTANGLE: return m_data.m_rectangle.Contact(other);

		default:
		{
			SDL_assert(false);
			std::cerr << "Unhandled shape combination";
			return 0.0f;
		}
	}
}