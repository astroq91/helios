#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	class TextureSampler
	{
	public:
		static SharedPtr<TextureSampler> create()
		{
			SharedPtr<TextureSampler> obj = SharedPtr<TextureSampler>::create();
            obj->init();
			return obj;
		}

		static std::unique_ptr<TextureSampler> create_unique()
		{
			std::unique_ptr<TextureSampler> obj = std::make_unique<TextureSampler>();
            obj->init();
			return obj;
		}

		const VkSampler& get_vk_sampler() const { return m_sampler; }

		~TextureSampler();
		TextureSampler() = default;

		TextureSampler(const TextureSampler&) = delete;
		TextureSampler& operator=(const TextureSampler&) = delete;
		TextureSampler(TextureSampler&&) = delete;
		TextureSampler& operator=(TextureSampler&&) = delete;

	private:
		void init();

	private:
		VkSampler m_sampler;

		bool m_is_initialized = false;
	};
}
