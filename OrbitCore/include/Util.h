/* @file Util.h */

#ifndef UTIL_H
#define UTIL_H
#pragma once

#include <array>
#include <fstream>
#include <vector>
#include <string>

#if defined(_WIN32)
#if defined(CORE)
#define ORBIT_CORE_API __declspec(dllexport)
#else
#define ORBIT_CORE_API __declspec(dllimport)
#endif
#else
#define ORBIT_CORE_API
#endif

namespace Orbit
{
	constexpr bool string_equal(char const* a, char const* b)
	{
		while (*a || *b)
			if (*a++ != *b++)
				return false;

		return true;
	}

	class const_str final
	{
	public:
		template<std::size_t N>
		constexpr const_str(const char(&a)[N]) : _cstr(a), _size(N - 1)
		{ }

		constexpr char operator[](std::size_t n)
		{
			return n < _size ? _cstr[n] : throw std::out_of_range("");
		}

		constexpr bool operator==(const const_str& rhs) const
		{
			return string_equal(_cstr, rhs._cstr);
		}

		constexpr bool operator!=(const const_str& rhs) const
		{
			return !operator==(rhs);
		}

		bool operator==(const std::string& rhs) const
		{
			return string_equal(_cstr, rhs.c_str());
		}

		bool operator!=(const std::string& rhs) const
		{
			return !operator==(rhs);
		}

		friend bool operator==(const std::string& lhs, const const_str& rhs)
		{
			return rhs == lhs;
		}

		friend bool operator!=(const std::string& lhs, const const_str& rhs)
		{
			return rhs != lhs;
		}

		constexpr std::size_t size() const 
		{
			return _size; 
		}

		const char* const c_str() const
		{
			return _cstr;
		}

	private:
		const char* const _cstr;
		const std::size_t _size;
	};

	template <typename T, typename... U>
	constexpr std::array<T, sizeof...(U)> make_array(U&&... u)
	{
		return { {std::forward<U>(u)...} };
	}

	inline std::vector<char> loadFile(std::string fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Could not open file " + fileName + "!");

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		return buffer;
	}
}

#endif //UTIL_H