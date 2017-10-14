/* @file Util.h */

#ifndef UTIL_H
#define UTIL_H
#pragma once

#include <array>
#include <fstream>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
		constexpr const_str() : const_str("") { }

		template<std::size_t N>
		constexpr const_str(const char(&a)[N]) : _cstr(a), _size(N - 1)
		{ }

		constexpr char operator[](std::size_t n)
		{
			return n < _size ? _cstr[n] : throw std::out_of_range("");
		}
		
		// Less operator required for set/map usage. Simply checks the pointers
		constexpr bool operator<(const const_str& rhs) const
		{
			return _cstr < rhs._cstr;
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

		constexpr size_t size() const 
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

	template<typename A, typename B>
	class bimap final
	{
	public:
		bimap() = default;
		~bimap() = default;

		bool try_insert(const A& a, const B& b)
		{
			if (_forwardMap.find(a) != _forwardMap.end())
				return false;

			_dataStorage.push_back({ a, b });
			std::pair<A, B>* newPair = &_dataStorage.back();

			_forwardMap[a] = newPair;
			_reverseMap[b] = newPair;

			return true;
		}
		
		B& operator[](const A& key)
		{
			try_insert(key, B());

			return _forwardMap[key]->second;
		}

		std::pair<B, bool> find(const A& key) const
		{
			auto found = _forwardMap.find(key);
			if (found == _forwardMap.end())
				return { B(), false };

			return { found->second->second, true };
		}

		A& operator[](const B& key)
		{
			try_insert(A(), key);
			
			return _reverseMap[key]->first;
		}

		std::pair<A, bool> find(const B& key) const
		{
			auto found = _reverseMap.find(key);
			if (found == _reverseMap.end())
				return { A(), false };

			return { found->second->first, true };
		}

	private:
		// List ensures memory addresses don't change.
		std::list<std::pair<A, B>> _dataStorage;
		std::unordered_map<A, std::pair<A, B>*> _forwardMap;
		std::unordered_map<B, std::pair<A, B>*> _reverseMap;
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

namespace std
{
	template<>
	struct hash<Orbit::const_str>
	{
		size_t operator()(const Orbit::const_str& str) const
		{
			return std::hash<std::string>()(str.c_str());
		}
	};
}

#endif //UTIL_H