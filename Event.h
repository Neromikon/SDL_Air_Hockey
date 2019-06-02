#pragma once

#include <functional>
#include <vector>


template<typename function_type>
class Event
{
public:
	void AddListener(std::function<function_type> listener)
	{
		m_listeners.push_back(listener);
	}

	void Invoke() const
	{
		for (auto listener : m_listeners)
		{
			listener();
		}
	}

	template<typename... Args>
	void Invoke(Args... args) const
	{
		for (auto listener : m_listeners)
		{
			listener(args...);
		}
	}

private:
	std::vector<std::function<function_type>> m_listeners;
};