/*! @file Input/Mouse.h */

#ifndef INPUT_MOUSE_H
#define INPUT_MOUSE_H
#pragma once

#include <iostream>

#include "Util.h"

#if defined(_WIN32)
#if defined(CORE)
#define ORBIT_API __declspec(dllexport)
#else
#define ORBIT_API __declspec(dllimport)
#endif
#endif

namespace Orbit
{
	class Mouse final
	{
	public:
		enum class Code : uint16_t
		{
			None,

			Mouse1, Mouse2, Mouse3, Mouse4, Mouse5,
		};

		constexpr static size_t count() { return names.size(); }

		constexpr Mouse(const Code& code) : _code(code) { }

		constexpr Mouse(const const_str& name)
		{
			for (size_t i = 0; i < names.size(); i++)
			{
				if (names[i] == name)
				{
					_code = static_cast<Code>(i);
					return;
				}
			}

			throw std::runtime_error("Mouse name not recognized!");
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

		constexpr bool operator==(const Mouse& rhs) const
		{
			return _code == rhs._code;
		}

		constexpr bool operator!=(const Mouse& rhs) const
		{
			return !operator==(rhs);
		}

		friend std::ostream& operator<<(std::ostream& o, const Mouse& button)
		{
			o << button.getName().c_str();
			return o;
		}

	private:
		constexpr const static auto names = make_array<const_str>(
			"None",

			"Mouse1", "Mouse2", "Mouse3", "Mouse4", "Mouse5");

		Code _code = Code::None;
	};
}

#endif //INPUT_MOUSE_H
