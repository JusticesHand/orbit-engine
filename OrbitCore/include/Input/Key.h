/*! @file Input/Key.h */

#ifndef INPUT_KEY_H
#define INPUT_KEY_H
#pragma once

#include <iostream>

#include "Util.h"

namespace Orbit
{
	/*!
	@brief Wrapper class for keys. Adds a layer of indirection that allows both OS and library indirection to virtual key codes.
	*/
	class Key final
	{
	public:
		/*!
		@brief Enumeration containing key codes. Instances of the class are essentially wrappers over this value.
		*/
		enum class Code : uint16_t
		{
			None,

			A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			Esc, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
			PrntScrn, ScrLk, Insert, Home, PgUp, PgDown, Delete, End,
			Up, Down, Left, Right,
			Tilde, N1, N2, N3, N4, N5, N6, N7, N8, N9, N0, Minus, Equal,
			Tab, CapsLock, LShift, LCtrl, LSuper, LAlt, RShift, RCtrl, RAlt, RSuper, Space,
			Backspace, Return, Backslash, Slash, Period, Comma, Semicolon, Apostrophe,
			LeftBracket, RightBracket,

			Mouse1, Mouse2, Mouse3, Mouse4, Mouse5
		};

		/*!
		@brief Returns the amount of keys available to the system. Useful when building arrays to poll.
		@return The amount of keys available.
		*/
		constexpr static size_t count() { return names.size(); }

		/*!
		@brief Default constructor for the class. Defaults to Orbit::Key::Code::None.
		*/
		constexpr Key() : Key(Code::None) { }

		/*!
		@brief Constructor for the class. Builds an instance of the class with the code in parameter.
		@param code The code to wrap in the class instance.
		*/
		constexpr Key(const Code& code) : _code(code) { }

		/*!
		@brief Constructor for the class. Statically verifies if the name is in the names array and translates the name to
		a code. Private constructor as it is used as part of a slightly more complex static assertion framework.
		When the key name corresponds to an actual key name, a static_assert will succeed (as the constructor will actually be
		evaluated as a constexpr expression). If the name doesn't exist, it will fail - use this to ensure constexpr evaluation.
		@param name The name of the key code.
		*/
		constexpr Key(const const_str& name)
		{
			for (size_t i = 0; i < names.size(); i++)
			{
				if (names[i] == name)
				{
					_code = static_cast<Code>(i);
					return;
				}
			}

			throw std::runtime_error("Key name not recognized!");
		}

		/*!
		@brief Returns the code representation of this instance.
		@return The key's code.
		*/
		Code getCode() const { return _code; }

		/*!
		@brief Returns a string representation of the key code.
		@return The key's name, as a const_str.
		*/
		constexpr const_str getName() const
		{
			return names[static_cast<size_t>(_code)];
		}

		/*!
		@brief Returns an index for accessing an array initialized with a size of Key::count().
		@see Key::count()
		@return The index representing the key.
		*/
		constexpr size_t index() const
		{
			return static_cast<size_t>(_code);
		}

		/*!
		@brief Equality operator for the class. Verifies if the keys are equal.
		@param rhs The right hand side of the operation.
		@return The result of the equality check.
		*/
		constexpr bool operator==(const Key& rhs) const
		{
			return _code == rhs._code;
		}

		/*!
		@brief Inequality operator for the class. Verifies if the keys are inequal.
		@param rhs The right hand side of the operation.
		@return The result of the equality check.
		*/
		constexpr bool operator!=(const Key& rhs) const
		{
			return !operator==(rhs);
		}

		/*!
		@brief Friend stream output operator allowing direct output of a Key to a standard std::ostream instance.
		@param o The stream on which to output.
		@param key The key to output on the stream.
		@return The (now modified) output stream.
		*/
		friend std::ostream& operator<<(std::ostream& o, const Key& key)
		{
			o << key.getName().c_str();
			return o;
		}

	private:
		/*! Compile-time constant array of names for keys. Intended for a one-to-one relation to the Code enum. */
		constexpr const static auto names = make_array<const_str>(
			"None",

			"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S",
			"T", "U", "V", "W", "X", "Y", "Z",
			"Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
			"PrntScrn", "ScrLk", "Insert", "Home", "PgUp", "PgDown", "Delete", "End",
			"Up", "Down", "Left", "Right",
			"Tilde", "N1", "N2", "N3", "N4", "N5", "N6", "N7", "N8", "N9", "N0", "Minus", "Equal",
			"Tab", "CapsLock", "LShift", "LCtrl", "LSuper", "LAlt", "RShift", "RCtrl", "RAlt", "RSuper", "Space",
			"Backspace", "Return", "Backslash", "Slash", "Period", "Comma", "Semicolon", "Apostrophe",
			"LeftBracket", "RightBracket",
			
			"Mouse1", "Mouse2", "Mouse3", "Mouse4", "Mouse5");

		/*! The key's code. */
		Code _code = Code::None;
	};
}

namespace std
{
	/*!
	@brief Extension hash specialization for the Orbit::Key class.
	*/
	template<>
	struct hash<Orbit::Key>
	{
		/*!
		@brief Returns the hashed value of the key.
		@return The hashed value of the key.
		*/
		size_t operator()(const Orbit::Key& value) const
		{
			return std::hash<Orbit::Key::Code>()(value.getCode());
		}
	};
}

#endif //INPUT_KEY_H