#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	struct ImageSpec
	{
		uint32_t width;
		uint32_t height;

		VkFormat format;
		VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage;
		VkMemoryPropertyFlags memory_property;
	};

	class Image
	{
	public:
		static Ref<Image> create(const ImageSpec& spec)
		{
			Ref<Image> obj = make_ref<Image>();
            obj->init(spec);
			return obj;
		}

		static Unique<Image> create_unique(const ImageSpec& spec)
		{
			Unique<Image> obj = make_unique<Image>();
            obj->init(spec);
			return obj;
		}

		VkImage get_vk_image() const { return m_image; }
		const VkImageView& get_vk_image_view() const { return m_image_view; }
		const VkDeviceMemory& get_vk_memory() const { return m_image_memory; }
		const VkDeviceSize& get_vk_size() const { return m_image_size; }

		uint32_t get_width() const { return m_width; }
		uint32_t get_height() const { return m_height; }

		~Image();
		Image() = default;

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(Image&&) = delete;

	private:
		void init(const ImageSpec& spec);

	private:
		VkImage m_image;
		VkImageView m_image_view;
		VkDeviceMemory m_image_memory;
		VkDeviceSize m_image_size;

		uint32_t m_width;
		uint32_t m_height;

		bool m_is_initialized = false;
	};
}
