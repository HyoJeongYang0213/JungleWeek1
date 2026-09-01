#pragma once

#include <random>
#include <type_traits>

namespace Rnd
{
	inline std::mt19937& GetEngine()
	{
		static thread_local std::mt19937 Engine{ std::random_device{}() };
		return Engine;
	}

	template <typename T>
	inline T GetRandom(T Min, T Max)
	{
		static_assert(std::is_arithmetic_v<T>, "Rnd::Get requires an arithmetic type.");

		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> Distribution{ Min, Max };
			return Distribution(GetEngine());
		}
		else
		{
			std::uniform_real_distribution<T> Distribution{ Min, Max };
			return Distribution(GetEngine());
		}
	}
}