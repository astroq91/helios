#pragma once
#include <map>
#include <memory>
#include <string>

#include "Texture.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	class TextureLibrary
	{
	public:
		void add_texture(const Ref<Texture>& texture);

		Ref<Texture> get_texture(const std::string& name);

	private:
		std::map<std::string, Ref<Texture>> m_textures;
	};
}
