/*! @file Render/Model.h */

#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H
#pragma once

#include "Util.h"

#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Orbit
{
	struct Vertex final
	{
		ORBIT_CORE_API Vertex(
			const glm::vec3& pos = glm::vec3(),
			const glm::vec2& uv = glm::vec2(),
			const glm::vec3& normal = glm::vec3(),
			const glm::vec4& color = glm::vec4());

		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec4 color;

		ORBIT_CORE_API bool operator==(const Vertex& rhs) const;

		ORBIT_CORE_API static size_t posOffset();
		ORBIT_CORE_API static size_t uvOffset();
		ORBIT_CORE_API static size_t normalOffset();
		ORBIT_CORE_API static size_t colorOffset();
		ORBIT_CORE_API static size_t size();
	};

	class Texture;

	class Model final
	{
	public:
		ORBIT_CORE_API Model(std::nullptr_t);
		ORBIT_CORE_API Model(const std::vector<Vertex>& vertexList, std::shared_ptr<const Texture> texture = nullptr);

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		ORBIT_CORE_API Model(Model&& rhs);
		ORBIT_CORE_API Model& operator=(Model&& rhs);

		ORBIT_CORE_API void setTexture(std::shared_ptr<const Texture> texture);

		ORBIT_CORE_API std::shared_ptr<const Texture> getTexture() const;

		ORBIT_CORE_API const std::vector<Vertex>& getVertices() const;
		ORBIT_CORE_API const std::vector<uint32_t>& getIndices() const;

		ORBIT_CORE_API size_t hash_code() const;

		ORBIT_CORE_API bool operator==(const Model& rhs) const;

	private:
		mutable size_t _calculatedHash = std::numeric_limits<size_t>::max();

		std::shared_ptr<const Texture> _texture = nullptr;
		std::vector<Vertex> _vertices;
		std::vector<uint32_t> _indices;
	};
}

namespace std
{
	template<>
	struct hash<Orbit::Vertex>
	{
		size_t operator()(const Orbit::Vertex& v) const
		{
			hash<glm::vec3> vec3hash;
			hash<glm::vec2> vec2hash;
			hash<glm::vec4> vec4hash;
			return (vec3hash(v.pos) << 12) ^
				(vec2hash(v.uv) << 8) ^
				(vec3hash(v.normal) << 4) ^
				vec4hash(v.color);
		}
	};

	template<>
	struct hash<Orbit::Model>
	{
		size_t operator()(const Orbit::Model& model) const
		{
			return model.hash_code();
		}
	};
}

#endif //RENDER_MODEL_H