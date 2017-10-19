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
	/*!
	@brief Class wrapping a vertex definition, consisting of position, uv, normal and color data (in that order).
	*/
	struct Vertex final
	{
		/*!
		@brief Constructor for the class. Initializes the vertex's members with those in parameter.
		@param pos The position of the vertex.
		@param uv The UV coordinates of the vertex (for use in texture mapping).
		@param normal The normal coordinates of the vertex.
		@param color The color of the vertex.
		*/
		ORBIT_CORE_API Vertex(
			const glm::vec3& pos = glm::vec3(),
			const glm::vec2& uv = glm::vec2(),
			const glm::vec3& normal = glm::vec3(),
			const glm::vec4& color = glm::vec4());

		/*! The position of the vertex. */
		glm::vec3 pos;
		/*! The UV coordinates of the vertex. */
		glm::vec2 uv;
		/*! The normal of the vertex. */
		glm::vec3 normal;
		/*! The color of the vertex. */
		glm::vec4 color;

		/*!
		@brief Equality test operator for the class. Verifies if each component is equal to the other vertex's.
		@param rhs The right hand side of the operation.
		@return The result of the comparison.
		*/
		ORBIT_CORE_API bool operator==(const Vertex& rhs) const;

		/*!
		@brief Returns the offset of the pos member of the class.
		@return The offset of the pos member of the class, in bytes.
		*/
		ORBIT_CORE_API static size_t posOffset();
		
		/*!
		@brief Returns the offset of the uv member of the class.
		@return The offset of the uv member of the class, in bytes.
		*/
		ORBIT_CORE_API static size_t uvOffset();

		/*!
		@brief Returns the offset of the normal member of the class.
		@return The offset of the normal member of the class, in bytes.
		*/
		ORBIT_CORE_API static size_t normalOffset();

		/*!
		@brief Returns the offset of the color member of the class.
		@return The offset of the color member of the class, in bytes.
		*/
		ORBIT_CORE_API static size_t colorOffset();

		/*!
		@brief Returns the total size of the class.
		@return The total size of the class, in bytes.
		*/
		ORBIT_CORE_API static size_t size();
	};

	class Texture;

	/*!
	@brief Class simplifying access to model data.
	*/
	class Model final
	{
	public:
		/*!
		@brief Default constructor of the class. Builds an empty model.
		*/
		ORBIT_CORE_API Model(std::nullptr_t);

		/*!
		@brief Parameter-based constructor of the class. Extracts vertices from the vertex list, and then builds an internal
		vertex-index representation of that list. Then simply assigns the texture to itself.
		@param vertexList The total list of vertices for the model.
		@param texture The texture to apply.
		*/
		ORBIT_CORE_API Model(const std::vector<Vertex>& vertexList, std::shared_ptr<const Texture> texture = nullptr);

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		/*!
		@brief Move constructor for the class. Moves the vertices and indices around, and copies the texture pointer.
		@param rhs The Model to move.
		*/
		ORBIT_CORE_API Model(Model&& rhs);

		/*!
		@brief Move assignment operator for the class. Moves the vertices and indices around, and copies the texture pointer.
		@param rhs The Model to move.
		*/
		ORBIT_CORE_API Model& operator=(Model&& rhs);

		/*!
		@brief Sets the texture pointer to that in parameter.
		@param texture The texture pointer to set.
		*/
		ORBIT_CORE_API void setTexture(std::shared_ptr<const Texture> texture);

		/*!
		@brief Getter for the texture pointer member.
		@return The texture pointer (nullptr if none).
		*/
		ORBIT_CORE_API std::shared_ptr<const Texture> getTexture() const;

		/*!
		@brief Getter for the model's vertices.
		@return A const reference to the model's vertices.
		*/
		ORBIT_CORE_API const std::vector<Vertex>& getVertices() const;

		/*!
		@brief Getter for the model's indices.
		@return A const reference to the model's indices.
		*/
		ORBIT_CORE_API const std::vector<uint32_t>& getIndices() const;

		/*!
		@brief Calculates a hash code for the model. As the hash code is (in most cases) complex to calculate, the result
		of the hash code is saved as a member of the class and returned immediately once computed at least once.
		@return The calculated hash code of the model.
		*/
		ORBIT_CORE_API size_t hash_code() const;

		/*!
		@brief Equality comparison operator for the class. To simplify and streamline operations, this isn't a strict equality
		comparator - it simply verifies if the hash codes of the models are equal. That way, the complexity of the operation is
		O(1) at best and O(vertices.size() + indices.size() + rhs.vertices.size() + rhs.indices.size()) at worst (only once).
		This optimization is especially useful when considering that there may exist maps where the models might be used as keys,
		and O(1) access is preferred.
		@param rhs The right hand side of the operation.
		@return The result of the comparison.
		*/
		ORBIT_CORE_API bool operator==(const Model& rhs) const;

	private:
		/*! The saved calculated hash for the model. If not computes, it equals std::numeric_limits<size_t>::max().*/
		mutable size_t _calculatedHash = std::numeric_limits<size_t>::max();

		/*! A pointer to the texture used by the model. */
		std::shared_ptr<const Texture> _texture = nullptr;
		/*! A coherent collection of vertices. */
		std::vector<Vertex> _vertices;
		/*! A coherent collection of indices. */
		std::vector<uint32_t> _indices;
	};
}

namespace std
{
	/*!
	@brief Extension of the hash template class to enable simple hashing of a Vertex.
	*/
	template<>
	struct hash<Orbit::Vertex>
	{
		/*!
		@brief Calculates the hash code of a vertex.
		@param v The vertex whose hash is requested.
		@return The result of the hashing operation.
		*/
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

	/*!
	@brief Extension of the hash template class to enable simple hashing of a Model.
	*/
	template<>
	struct hash<Orbit::Model>
	{
		/*!
		@brief Simply calls the model's hash_code function.
		@return The result of the Model's hash_code function.
		@see Orbit::Model::hash_code()
		*/
		size_t operator()(const Orbit::Model& model) const
		{
			return model.hash_code();
		}
	};
}

#endif //RENDER_MODEL_H