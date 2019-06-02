#pragma once


template <typename type>
struct Resource
{
	type* const data;
	std::string file;

	inline Resource(type &data, const std::string& file):
		data(&data),
		file(file)
	{}

	inline type& operator= (const type& other) const
	{
		*data = other;
		return *data;
	}

	inline bool operator== (const type& other) const
	{
		return *data == other;
	}
};