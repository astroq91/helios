#pragma once
#include <vulkan/vulkan.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	class CommandBuffer
	{
	public:
		static Ref<CommandBuffer> create()
		{
			Ref<CommandBuffer> cb = make_ref<CommandBuffer>();
            cb->init();
			return cb;
		}

		static Unique<CommandBuffer> create_unique()
		{
			Unique<CommandBuffer> cb = make_unique<CommandBuffer>();
            cb->init();
			return cb;
		}

		const VkCommandBuffer& get_command_buffer() const { return m_buffer; }

		CommandBuffer() = default;
		~CommandBuffer() = default;

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) = delete;

	private:
		void init();

	private:
		VkCommandBuffer m_buffer;
	};
}
