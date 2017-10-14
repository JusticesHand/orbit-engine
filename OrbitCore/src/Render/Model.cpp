/*! @file Render/Model.cpp */

#include "Render/Model.h"

#include <cstddef>
#include <unordered_map>

using namespace Orbit;

Vertex::Vertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec3& normal, const glm::vec4& color)
	: pos(pos), uv(uv), normal(normal), color(color)
{
}

bool Vertex::operator==(const Vertex& rhs) const
{
	return pos == rhs.pos && uv == rhs.uv && normal == rhs.normal && color == rhs.color;
}

size_t Vertex::posOffset()
{
	return offsetof(Vertex, pos);
}

size_t Vertex::uvOffset()
{
	return offsetof(Vertex, uv);
}

size_t Vertex::normalOffset()
{
	return offsetof(Vertex, normal);
}

size_t Vertex::colorOffset()
{
	return offsetof(Vertex, color);
}

size_t Vertex::size()
{
	return sizeof(Vertex);
}

Model::Model(std::nullptr_t)
{ 
}

Model::Model(const std::vector<Vertex>& vertexList, std::shared_ptr<const Texture> texture)
	: _texture(texture)
{
	std::unordered_map<Vertex, uint32_t> vertexIndices;
	for (const Vertex& vertex : vertexList)
	{
		if (vertexIndices.find(vertex) == vertexIndices.end())
		{
			vertexIndices[vertex] = static_cast<uint32_t>(_vertices.size());
			_vertices.push_back(vertex);
		}

		_indices.push_back(vertexIndices[vertex]);
	}
}

Model::Model(Model&& rhs)
	: _texture(rhs._texture), _vertices(std::move(rhs._vertices)), _indices(std::move(rhs._indices))
{
}

Model& Model::operator=(Model&& rhs)
{
	_texture = rhs._texture;
	_vertices = std::move(rhs._vertices);
	_indices = std::move(rhs._indices);
	return *this;
}

void Model::setTexture(std::shared_ptr<const Texture> texture)
{
	_texture = texture;
}

std::shared_ptr<const Texture> Model::getTexture() const
{
	return _texture;
}

const std::vector<Vertex>& Model::getVertices() const
{
	return _vertices;
}

const std::vector<uint32_t>& Model::getIndices() const
{
	return _indices;
}

size_t Model::hash_code() const
{
	// The model won't change in its lifetime. Therefore, the hash should only be calculated once
	// (as the models might be very, very large)
	if (_calculatedHash != std::numeric_limits<size_t>::max())
		return _calculatedHash;

	// Essentially boost::hash_range algorithm. Golden number is pretty interesting in theory (effectively being 'random' bits)
	size_t seed = 0;

	std::hash<Vertex> vertexHasher;
	for (const Vertex& v : _vertices)
		seed ^= vertexHasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

	std::hash<uint32_t> indexHasher;
	for (const uint32_t& i : _indices)
		seed ^= indexHasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

	_calculatedHash = seed;
	return _calculatedHash;
}

bool Model::operator==(const Model& rhs) const
{
	return hash_code() == rhs.hash_code();
}