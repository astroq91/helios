#pragma once
#include "DescriptorSetLayout.h"
#include "Shader.h"
#include "VertexBufferDescription.h"

namespace Helios {

struct PipelineCreateInfo {
    VkFormat color_attachment_format;
    const std::vector<Ref<DescriptorSetLayout>>& descriptor_set_layouts;
    const Ref<Shader>& vertex_shader;
    const Ref<Shader>& fragment_shader;
    const std::vector<VertexBufferDescription>& vertex_buffer_descriptions;
    const std::vector<VkPushConstantRange> push_constants = {};
    const std::vector<VkPipelineColorBlendAttachmentState>&
        color_blend_attachments = {};
};

class Pipeline {
  public:
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
    static Unique<Pipeline> create_unique(const PipelineCreateInfo& info) {
        Unique<Pipeline> pl = make_unique<Pipeline>();
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
