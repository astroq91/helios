#pragma once
#include "DescriptorSetLayout.h"
#include "Helios/Core/Core.h"
#include "Shader.h"
#include "VertexBufferDescription.h"

namespace Helios {

struct PipelineCreateInfo {
    VkFormat color_attachment_format;
    std::vector<SharedPtr<DescriptorSetLayout>> descriptor_set_layouts;
    SharedPtr<Shader> vertex_shader;
    SharedPtr<Shader> fragment_shader;
    std::vector<VertexBufferDescription> vertex_buffer_descriptions;
    std::vector<VkPushConstantRange> push_constants = {};
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments =
        {};
};

class Pipeline {
  public:
    static SharedPtr<Pipeline> create(const PipelineCreateInfo& info) {
        SharedPtr<Pipeline> pl = SharedPtr<Pipeline>::create();
        pl->init(info);
        return pl;
    }
    /**
     * \brief create a new Pipeline.
     * \param color_attachment_format The color attachment format.
     * \param descriptor_set_layouts All the layouts of the different types of
     * descriptor sets.
     * \param vertex_shader The vertex shader.
     * \param fragment_shader The fragment shader.
     * \param vertex_buffer_descriptions The vertex buffers' descriptions. Used
     * for vertex attributes.
     * \param push_constants Optional push constants.
     */
    static std::unique_ptr<Pipeline> create_unique(const PipelineCreateInfo& info) {
        std::unique_ptr<Pipeline> pl = std::make_unique<Pipeline>();
        pl->init(info);
        return pl;
    }

    VkPipeline get_vk_pipeline() const { return m_pipeline; }
    VkPipelineLayout get_vk_layout() const { return m_layout; }

    Pipeline() = default;
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

  private:
    void init(const PipelineCreateInfo& info);

  private:
    VkPipeline m_pipeline;
    VkPipelineLayout m_layout;

    bool m_is_initialized = false;
};
} // namespace Helios
