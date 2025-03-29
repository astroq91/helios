#pragma once
#include "Helios/Core/Layer.h"
#include "Helios/Renderer/DescriptorPool.h"

namespace Helios
{
	class ImGuiLayer : public Layer
	{
	public:
		void on_attach() override;
		void on_detach() override;
		void on_event(Event& e) override;

		~ImGuiLayer() override;


		void begin();
		void end();

		void set_block_events(bool state) { m_block_events = state; }

	private:
		bool m_block_events = true;

		std::unique_ptr<DescriptorPool> m_descriptor_pool;
	};
}
