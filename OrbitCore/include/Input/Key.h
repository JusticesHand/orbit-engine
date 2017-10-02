/*! @file Input/Key.h */

#ifndef INPUT_KEY_H
#define INPUT_KEY_H
#pragma once

#include <iostream>

#include "Util.h"

namespace Orbit
{
	class Key final
	{
	public:
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
			LeftBracket, RightBracket
		};

		constexpr static size_t count() { return names.size(); }

		constexpr Key(const Code& code) : _code(code) { }

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

		Code getCode() const { return _code; }

		constexpr const_str getName() const
		{
			return names[static_cast<size_t>(_code)];
		}

		constexpr size_t index() const
		{
			return static_cast<size_t>(_code);
		}

		constexpr bool operator==(const Key& rhs) const
		{
			return _code == rhs._code;
		}

		constexpr bool operator!=(const Key& rhs) const
		{
			return !operator==(rhs);
		}

		friend std::ostream& operator<<(std::ostream& o, const Key& key)
		{
			o << key.getName().c_str();
			return o;
		}

	private:
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
			"LeftBracket", "RightBracket");

		Code _code = Code::None;
	};
}

#endif //INPUT_KEY_H