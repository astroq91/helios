#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "Transform.h"

namespace Helios
{
	class Camera
	{
	public:
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 view_projection_matrix;

		glm::vec3 position;

	public:
		Camera() = default;

		// Fov in degrees (not radians)
		Camera(const Transform& transform, float aspect_ratio, float fovY, float _near, float _far);
	};
}
