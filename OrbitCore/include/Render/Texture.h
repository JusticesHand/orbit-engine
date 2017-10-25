/*! @file Render/Texture.h */

#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Util.h"

namespace Orbit
{
	/*!
	@brief Class abstracting texture loading and storing operations.
	*/
	class Texture final
	{
	public:
		/*!
		@brief Constructor for the class. Builds an empty texture.
		*/
		ORBIT_CORE_API Texture(std::nullptr_t);

		/*!
		@brief Constructor for the class. Builds a texture from the file pointed to by name.
		@param name The name of the texture to load.
		*/
		ORBIT_CORE_API explicit Texture(const std::string& name);

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		/*!
		@brief Move constructor for the class. Moves the texture's assets from the right hand side.
		@param rhs The right hand side of the operation.
		*/
		ORBIT_CORE_API Texture(Texture&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves the texture's assets from the right hand side.
		@param rhs The right hand side of the operation.
		@return A reference to this.
		*/
		ORBIT_CORE_API Texture& operator=(Texture&& rhs);

		/*!
		@brief Getter for the size of the texture.
		@return The size of the texture.
		*/
		ORBIT_CORE_API glm::ivec2 size() const;

		/*!
		@brief Getter for the byte data of the texture, contained in a vector for simplicity.
		@return The byte data of the texture.
		*/
		ORBIT_CORE_API const std::vector<uint8_t>& data() const;

	private:
		/*! The size of the texture. */
		glm::ivec2 _texSize;
		/*! The actual data. */
		std::vector<uint8_t> _bytes;
	};
}

#endif //RENDER_TEXTURE_H