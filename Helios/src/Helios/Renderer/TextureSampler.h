#pragma once

#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	class TextureSampler
	{
	public:
		static Ref<TextureSampler> create()
		{
			Ref<TextureSampler> obj = make_ref<TextureSampler>();
            obj->init();
			return obj;
		}

		static Unique<TextureSampler> create_unique()
		{
			Unique<TextureSampler> obj = make_unique<TextureSampler>();
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
