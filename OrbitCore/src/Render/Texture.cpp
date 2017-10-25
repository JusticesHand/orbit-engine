/*! @file Render/Texture.cpp */

#include "Render/Texture.h"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Orbit;

Texture::Texture(std::nullptr_t)
{
}

Texture::Texture(const std::string& name)
{
	if (!std::ifstream(name).is_open())
		throw std::runtime_error("File " + name + " was not found!");

	int width, height, channels;
	stbi_uc* pix = stbi_load(name.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	_texSize = glm::ivec2{ width, height };

	size_t totalSize = width * height * 4;
	_bytes.resize(totalSize);
	memcpy(_bytes.data(), pix, totalSize);

	stbi_image_free(pix);
}

Texture::Texture(Texture&& rhs)
	: _texSize(rhs._texSize),
	_bytes(std::move(rhs._bytes))
{
}

Texture& Texture::operator=(Texture&& rhs)
{
	_texSize = rhs._texSize;
	_bytes = std::move(rhs._bytes);

	return *this;
}

glm::ivec2 Texture::size() const
{
	return _texSize;
}

const std::vector<uint8_t>& Texture::data() const
{
	return _bytes;
}