#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>


namespace Helios
{
	struct Vertex
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 normal;
		alignas(8) glm::vec2 tex_coords;


		bool operator==(const Vertex& other) const
		{
			return position == other.position && normal == other.normal && tex_coords == other.tex_coords;
		}
	};
}

namespace std
{
	template <>
	struct hash<Helios::Vertex>
	{
		size_t operator()(Helios::Vertex const& vertex) const noexcept
		{
			return ((hash<glm::vec3>()(vertex.position) ^
                     (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.tex_coords) << 1);
		}
	};
}
