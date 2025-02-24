#pragma once
#include <glm/glm.hpp>

namespace Helios
{
	struct DirectionalLight
	{
		alignas(16) glm::vec3 direction = glm::vec3(1.0f); // From the light source
		alignas(16) glm::vec3 ambient = glm::vec3(1.0f);
		alignas(16) glm::vec3 diffuse = glm::vec3(1.0f);
		alignas(16) glm::vec3 specular = glm::vec3(0.0f);
	};

	struct PointLight
	{
		alignas(16) glm::vec3 position = glm::vec3(0.0f);

		// Co-efficients for the fading of the point light, with respect to distance.
		alignas(4) float constant = 1.0f;
		alignas(4) float linear = 0.09f;
		alignas(4) float quadratic = 0.032f;

		alignas(16) glm::vec3 ambient = glm::vec3(1.0f);
		alignas(16) glm::vec3 diffuse = glm::vec3(1.0f);
		alignas(16) glm::vec3 specular = glm::vec3(1.0f);
	};
}
