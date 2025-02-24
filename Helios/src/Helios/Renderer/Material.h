#pragma once
#include "Texture.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	struct Material
	{
		Ref<Texture> diffuse = nullptr;
		Ref<Texture> specular = nullptr;
		Ref<Texture> emission = nullptr;
		float shininess = 32.0f;
	};
}
