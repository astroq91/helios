#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	struct SwapChain
	{
	public:
		static Ref<SwapChain> create()
		{
			Ref<SwapChain> sc = make_ref<SwapChain>();
            sc->init();
			return sc;
		}

		/**
		 * \brief create a new SwapChain.
		 */
		static Unique<SwapChain> create_unique()
		{
			Unique<SwapChain> sc = make_unique<SwapChain>();
            sc->init();
			return sc;
		}

		const VkSwapchainKHR& get_vk_swapchain() const { return m_swapchain; }
		const VkFormat& get_vk_format() const { return m_image_format; }
		size_t get_image_count() const { return m_images.size(); }
		const VkExtent2D& get_vk_extent() const { return m_extent; }

		const VkImageView& get_vk_image_view(size_t index) const { return m_image_views[index]; }
		const VkImage& get_vk_image(size_t index) const { return m_images[index]; }

		VkPresentModeKHR get_present_mode() const { return m_present_mode; }

		~SwapChain();
		SwapChain() = default;

		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;

	private:
		void init();

	private:
		VkSwapchainKHR m_swapchain;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_image_views;
		VkFormat m_image_format;
		VkExtent2D m_extent;

		VkPresentModeKHR m_present_mode;
		VkSurfaceFormatKHR m_surface_format;

		bool m_is_initialized = false;
	};
}
