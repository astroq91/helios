#pragma once
#include <volk/volk.h>

#include "Helios/Core/Core.h"

namespace Helios
{
	class CommandBuffer
	{
	public:
		static SharedPtr<CommandBuffer> create()
		{
			SharedPtr<CommandBuffer> cb = SharedPtr<CommandBuffer>::create();
            cb->init();
			return cb;
		}

		static std::unique_ptr<CommandBuffer> create_unique()
		{
			std::unique_ptr<CommandBuffer> cb = std::make_unique<CommandBuffer>();
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
