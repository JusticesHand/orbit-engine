/*! @file Util.h */

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

/*! Definition of import/export semantics for the core. Unnecessary if not under Windows. */
#if defined(_WIN32)
#if defined(CORE)
#define ORBIT_CORE_API __declspec(dllexport)
#else
#define ORBIT_CORE_API __declspec(dllimport)
#endif
#else
#define ORBIT_CORE_API
#endif

/*! Definition of import/export semantics. Unnecessary if not under Windows. */
#if defined(_WIN32)
#if defined(ORBIT)
#define ORBIT_API __declspec(dllimport)
#else
#define ORBIT_API __declspec(dllexport)
#endif
#else
#define ORBIT_API
#endif

/*! Definition of debug assertion macro. Does not compile check on release. */
#if defined(_DEBUG)
#define ASSERT_DEBUG(test, message) if (!test) throw std::runtime_error(message)
#else
#define ASSERT_DEBUG(test, message)
#endif

namespace Orbit
{
	/*!
	@brief Simple string equality comparison that assumes each string is properly null-terminated.
	This function is meant to be used as a drop-in replacement for strcmp where compile-time comparison
	is required, as it is a constexpr function that will be evaluated at compile time. Especially useful
	when taken in the context of the Orbit::const_str class.
	Note that it is about as safe as the regular strcmp function - as such, at runtime it is preferred to
	use the std::string class to comapre (and manipulate) strings.
	@see Orbit::const_str
	@param a The first string to compare.
	@param b The second string to compare.
	@return The result of the string equality comparison.
	*/
	inline constexpr bool string_equal(const char* a, const char* b)
	{
		while (*a || *b)
			if (*a++ != *b++)
				return false;

		return true;
	}

	/*!
	@brief Simple constant string class contributing safe, compile-time string operations to simplify tasks
	where such operations are required, such as where hard-coded names are appropriate.
	*/
	class const_str final
	{
	public:
		/*!	@brief Default constructor for the class. Builds an empty string. */
		constexpr const_str() : const_str("") { }

		/*!
		@brief Constructor for the class. Builds an instance that uses the string in parameter.
		Because of how the constructor is built, it is not usable with runtime strings (const char*).
		As it is not explicit, it supports implicit construction.
		@tparam N The size of the string, including the null character.
		@param a The string to use in construction.
		*/
		template<std::size_t N>
		constexpr const_str(const char(&a)[N]) : _cstr(a), _size(N - 1)
		{ }

		/*!
		@brief Random access operator to retrieve characters in the string. As a constexpr function, returns a constant
		value if the index in parameter is a compile-time constant (and ensures compile-time boundary checks in this case).
		@param n The index of the character to retrieve.
		@return The character at the index.
		@throw std::out_of_range Thrown when the index is over or equal to the string's size.
		*/
		constexpr char operator[](std::size_t n)
		{
			return n < _size ? _cstr[n] : throw std::out_of_range("Cannot return a character at this index!");
		}
		
		/*!
		@brief Less than operator used for map and set ordering comparisons. Simply returns an arithmetic comparison between
		the internal pointers - no proper string less-than operations are implemented here, as this is not its purpose.
		@param rhs The right hand side of the operation.
		@return Whether or not the internal pointer is arithmetically less than the other's pointer.
		*/
		constexpr bool operator<(const const_str& rhs) const
		{
			return _cstr < rhs._cstr;
		}

		/*!
		@brief Equality operator for the class. Executes a static, compile-time comparison of this and the const_str
		in rhs.
		@param rhs The right hand side of the operation, as a const_str.
		@return Whether or not the two strings are equal.
		*/
		constexpr bool operator==(const const_str& rhs) const
		{
			return string_equal(_cstr, rhs._cstr);
		}

		/*!
		@brief Inequality operator for the class. Simply calls the equality operator and inverts the result.
		@param rhs The right hand side of the operation, as a const_str.
		@return Whether or not the two string are inequal.
		*/
		constexpr bool operator!=(const const_str& rhs) const
		{
			return !operator==(rhs);
		}

		/*!
		@brief Equality operator for the class. Does a string comparison between the internal pointer and the std::string's
		c_str() pointer. As an std::string is not a compile-time constant, this comparison is not either and results of
		the comparison are not available at compile time.
		@param rhs The right hand side of the operation, as an std::string.
		@return Whether or not the two string are equal.
		*/
		bool operator==(const std::string& rhs) const
		{
			return string_equal(_cstr, rhs.c_str());
		}

		/*!
		@brief Inequality operator for the class. Simply calls the equality operator and inverts the result.
		@param rhs The right hand side of the operation, as an std::string.
		@return Whether or not the two strings are inequal.
		*/
		bool operator!=(const std::string& rhs) const
		{
			return !operator==(rhs);
		}

		/*!
		@brief Friend operator allowing the inversion of the parameter order for const_str equality checking with an std::string.
		@param lhs The std::string to compare.
		@param rhs The const_str to compare.
		@return The result of the comparison.
		*/
		friend bool operator==(const std::string& lhs, const const_str& rhs)
		{
			return rhs == lhs;
		}

		/*!
		@brief Friend operator allowing the inversion of the parameter order for const_str inequality checking with an std::string.
		@param lhs The std::string to compare.
		@param rhs The const_str to compare.
		@return The result of the comparison.
		*/
		friend bool operator!=(const std::string& lhs, const const_str& rhs)
		{
			return rhs != lhs;
		}

		/*!
		@brief Constant function returning the string's size.
		@return The string's size.
		*/
		constexpr size_t size() const 
		{
			return _size; 
		}

		/*!
		@brief Returns a pointer to the inner string, for interaction with c-like APIs.
		@return A pointer to the inner string.
		*/
		const char* c_str() const
		{
			return _cstr;
		}

	private:
		/*! A pointer to the inner string. */
		const char* const _cstr;
		/*! The string's size. */
		const std::size_t _size;
	};
	
	/*!
	@brief Class implementing a birectional map-like structure. As both types are keys and values, they must both have hashers
	(since the underlying map type is a regular std::unordered_map, which requires hashers).
	@tparam A The first type of the bidirectional map.
	@tparam B The second type of the bidirectional map.
	@tparam AHash The type of hasher to hash a value of type A.
	@tparam BHash The type of hasher to hash a value of type B.
	*/
	template<typename A, typename B, typename AHash = std::hash<A>, typename BHash = std::hash<B>>
	class bimap final
	{
	public:
		bimap() = default;
		~bimap() = default;

		/*!
		@brief Attempts an insertion of a key/value pair into the map. Should the pair exist, it exits silently and
		no insertion occurs.
		@param a The first element of the pair to insert.
		@param b The second element of the pair to insert.
		@return The result of the insertion.
		*/
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
		
		/*!
		@brief Access operator for the map. If not found, results in an insertion (same behaviour as std::unordered_map).
		@param key The key for which to search.
		@return A reference to the member in the map corresponding to the key.
		*/
		B& operator[](const A& key)
		{
			try_insert(key, B());

			return _forwardMap[key]->second;
		}

		/*!
		@brief Searches the map for a key. Returns a pair containing the found value and the result of the search.
		@param key The key for which to search.
		@return A pair containing the found value (if found, default if not) and the result of the search attempt.
		*/
		std::pair<B, bool> find(const A& key) const
		{
			auto found = _forwardMap.find(key);
			if (found == _forwardMap.end())
				return { B(), false };

			return { found->second->second, true };
		}

		/*!
		@brief Access operator for the map. If not found, results in an insertion (same behaviour as std::unordered_map).
		@param key The key for which to search.
		@return A reference to the member in the map corresponding to the key.
		*/
		A& operator[](const B& key)
		{
			try_insert(A(), key);
			
			return _reverseMap[key]->first;
		}

		/*!
		@brief Searches the map for a key. Returns a pair containing the found value and the result of the search.
		@param key The key for which to search.
		@return A pair containing the found value (if found, default if not) and the result of the search attempt.
		*/
		std::pair<A, bool> find(const B& key) const
		{
			auto found = _reverseMap.find(key);
			if (found == _reverseMap.end())
				return { A(), false };

			return { found->second->first, true };
		}

	private:
		/*! List containing the actual data storage. List ensures memory addresses do not change (as there is no resizing). */
		std::list<std::pair<A, B>> _dataStorage;
		/*! Forward map, mapping for keys of type A. */
		std::unordered_map<A, std::pair<A, B>*, AHash> _forwardMap;
		/*! Reverse map, mapping for keys of type B. */
		std::unordered_map<B, std::pair<A, B>*, BHash> _reverseMap;
	};

	/*!
	@brief Utility function allowing for constexpr arrays of constexpr elements. Useful when making constant arrays of elements
	(like, for example, a list of names for keys).
	@tparam T The type of the array's elements.
	@tparam U Template parameter pack expanded to give size to the array. Meant to be implied and never specified.
	@param u The list of the array's contents.
	@return The completed std::array containing the elements.
	*/
	template <typename T, typename... U>
	inline constexpr std::array<T, sizeof...(U)> make_array(U&&... u)
	{
		return { {std::forward<U>(u)...} };
	}

	/*!
	@brief Utility function returning the contents of a file as a std::vector<char> representing the file's data (in bytes).
	@param fileName The URI of the file to open.
	@return The byte contents of the file.
	*/
	inline std::vector<char> loadFile(const std::string& fileName)
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
	/*!
	@brief Extension of std::hash to be able to hash Orbit::const_str instances for compatibility with STL containers
	requiring a hash, such as std::unordered_map and std::unordered_set.
	*/
	template<>
	struct hash<Orbit::const_str>
	{
		/*!
		@brief Computes the hash of a Orbit::const_str. Simply computes the hash as if the const_str were a std::string -
		as such, it is not constant in time.
		@param str The string to hash.
		@return The result of the hash operation.
		*/
		size_t operator()(const Orbit::const_str& str) const
		{
			return std::hash<std::string>()(str.c_str());
		}
	};
}

#endif //UTIL_H