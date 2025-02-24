#pragma once
#include "KeyCodes.h"

#include <glm/glm.hpp>

namespace Helios
{
	class Input
	{
	public:
		static bool is_key_pressed(KeyCode key);
		static bool is_key_released(KeyCode key);

		static glm::vec2 get_mouse_pos();
	};
}
