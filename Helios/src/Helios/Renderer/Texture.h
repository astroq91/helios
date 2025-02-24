#pragma once

#include <vulkan/vulkan.h>
#include <string>

#include "DescriptorSet.h"
#include "Image.h"
#include "Helios/Assets/Asset.h"
#include "Helios/Core/Core.h"

namespace Helios
{
	class Texture : public Asset
	{
	public:
		/**
		 * \brief create a texture from path.
		 * \param path The path.
		 * \return The texture
		 */
		static Ref<Texture> create(const std::string& path)
		{
			Ref<Texture> obj = make_ref<Texture>();
            obj->init_asset(path);
            obj->init(path);
			return obj;
		}

		/**
		 * \brief create a texture from data.
		 * \param name The name of the texture.
		 * \param data The raw data.
		 * \param width The image width.
		 * \param height The image height.
		 * \param size The size (in bytes) of the image.
		 * \return The texture
		 */
		static Ref<Texture> create(const std::string& name, void* data, uint32_t width, uint32_t height, size_t size)
		{
			Ref<Texture> obj = make_ref<Texture>();
            obj->init_asset(name);
            obj->init(data, width, height, size);
			return obj;
		}

		const Ref<Image>& get_image() const { return m_image; }

		int32_t GetTextureIndex() const { return m_texture_index; }

		Texture() = default;
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) = delete;
		Texture& operator=(Texture&&) = delete;

	private:
		void init(const std::string& path);
		void init(void* data, uint32_t width, uint32_t height, size_t size);

	private:
		int32_t m_texture_index;

		Ref<Image> m_image;
	};
}
