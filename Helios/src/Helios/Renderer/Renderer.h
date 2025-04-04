#pragma once

#include <glm/glm.hpp>

#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "FontLibrary.h"
#include "Helios/Scene/PerspectiveCamera.h"
#include "Helios/Scene/Transform.h"
#include "Helios/Vulkan/VulkanContext.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Pipeline.h"
#include "Shader.h"
#include "ShaderLibrary.h"
#include "SwapChain.h"
#include "Texture.h"
#include "TextureLibrary.h"
#include "TextureSampler.h"
#include "UniformBuffer.h"

#include <freetype/freetype.h>

namespace Helios {
struct BeginRenderingAttachmentSpec {};

struct BeginRenderingSpec {
    SharedPtr<Image> color_image = nullptr; // Optional
    VkImageLayout color_image_layout = VK_IMAGE_LAYOUT_GENERAL;
    VkAttachmentLoadOp color_load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp color_store_op = VK_ATTACHMENT_STORE_OP_STORE;
    glm::vec4 color_clear_value = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    // Only used if VK_ATTACHMENT_LOAD_OP_CLEAR is used.

    SharedPtr<Image> depth_image = nullptr; // Optional
    VkAttachmentLoadOp depth_load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
    VkAttachmentStoreOp depth_store_op = VK_ATTACHMENT_STORE_OP_STORE;

    float depth_clear_value = 0.0f;
    uint32_t stencil_clear_value = 0;

    uint32_t width = 0;
    uint32_t height = 0;
};

struct PushConstantInfo {
    size_t size = 0;
    VkShaderStageFlags stages;
    std::vector<uint8_t> data;
};

struct CustomMeshPipelineInfo {
    SharedPtr<Pipeline> pipeline = nullptr;
    std::vector<SharedPtr<DescriptorSet>> descriptor_sets = {};
    PushConstantInfo push_constants = {};
};

// Used to store the geometries used for a set of instances.
struct MeshInstances {
    SharedPtr<Mesh> mesh;
    CustomMeshPipelineInfo custom_pipeline_info;
    size_t offset;
    size_t instance_count;
};

// Used to describe an instance's properties.
struct MeshRenderingInstance {
    Transform transform;
    SharedPtr<Material> material;
    glm::vec4 tint_color;
};

// Used for the texture indices
struct ShaderMaterial {
    alignas(4) int32_t diffuse_texture_unit;
    alignas(4) int32_t specular_texture_unit;
    alignas(4) int32_t emission_texture_unit;
    alignas(4) float shininess;
};

// Used as per-instance vertex attributes when rendering geometries.
struct MeshRenderingShaderInstanceData {
    alignas(16) glm::mat4 model;
    ShaderMaterial material;
    glm::vec4 tint_color;
};

struct QuadRenderingInstance {
    glm::mat4 model;
    glm::vec4 tint_color;
    int32_t texture_unit;
};

struct UIQuadShaderInstanceData {
    alignas(16) glm::mat4 model;
    alignas(16) glm::vec4 tint_color;
    alignas(4) int32_t texture_unit;
};

constexpr int k_max_meshes = 10000;
constexpr int k_max_textures = 1000;
constexpr int k_max_ui_quads = 10000;

constexpr int k_max_directional_lights = 32;
constexpr int k_max_point_lights = 32;

class Renderer {
  public:
    void init(uint32_t max_frames_in_flight);
    Renderer() = default;

    void shutdown();

    /**
     * \brief begin *recording* a new render pass.
     * \param spec An optional specification for a custom color attachment.
     * If none is given, one of the swap chain images are used.
     */
    void begin_rendering(const BeginRenderingSpec& spec = {});
    /**
     * \brief Stop *recording* the render pass.
     * \param submit If the recording should be submitted. Beware this waits, on
     * the CPU, for the completion of the commands.
     */
    void end_rendering();

    /**
     * \brief Submit the instances recorded so far. This will also record, and
     * clear, the specified lighting so far. \param begin_rendering_spec
     * Optional render pass specification.
     */
    void
    submit_mesh_instances(const BeginRenderingSpec& begin_rendering_spec = {});

    void submit_ui_quad_instances(
        const BeginRenderingSpec& begin_rendering_spec = {});

    /**
     * \brief Submit the current command buffer, and then wait for completion.
     * After that, it will start recording again.
     */
    void submit_command_buffer();

    /**
     * \brief [Helios internal] begin recording the global command buffer.
     * \param clear_attachment If the color and depth attachment should be
     * cleared.
     */
    void begin_recording(bool clear_attachment = true);
    /**
     * \brief [Helios internal] Stop recording the global command buffer.
     */
    void end_recording();

    /**
     * \brief [Helios internal] begin a new frame.
     */
    void begin_frame();
    /**
     * \brief [Helios internal] end the frame. This will submit the global
     * command buffer and present the current swap chain image.
     */
    void end_frame();

    /**
     * \brief [Helios internal] Registers a new texture, so it can be used in
     * shaders. \param texture The texture. \return The texture index.
     */
    int32_t register_texture(const Texture& texture);
    void deregister_texture(uint32_t textureIndex);

    void draw_ui_quad(const Transform& transform, const glm::vec4& color,
                      const SharedPtr<Texture>& texture = nullptr);

    /**
     * \brief Record cube instances.
     * \param instances The instances.
     */
    void draw_cube(const std::vector<MeshRenderingInstance>& instances);

    void draw_mesh(const SharedPtr<Mesh>& geometry,
                   const std::vector<MeshRenderingInstance>& instances,
                   const CustomMeshPipelineInfo& custom_pipeline = {});

    void set_perspective_camera(const PerspectiveCamera& camera);

    void render_directional_light(const DirectionalLight& dir_light);
    void render_point_light(const PointLight& point_light);

    void render_text(const std::string& text, const glm::vec2& position,
                     float scale, const glm::vec4& tint_color);

    void set_ui_projection_matrix(const glm::mat4& proj) {
        m_ui_projection = proj;
    }

    /**
     * \brief get a texture.
     * \param key The texture identifier.
     * \return The texture object (nullptr if the texture does not exist).
     */
    SharedPtr<Texture> get_texture(const std::string& key);

    const SharedPtr<SwapChain>& get_swapchain() const { return m_swapchain; }

    bool swapchain_recreated_this_frame() const {
        return m_swapchain_recreated_this_frame;
    }

    void recreate_swapchain_next_frame(bool vsync) {
        m_recreate_swapchain_next_frame = true;
        m_vsync = vsync;
    }

    bool vsync_enabled() const { return m_vsync; }

    void set_framebuffer_resized(bool value) { m_framebuffer_resized = value; }

    /**
     * \brief get the command buffer for this frame.
     * \return The command buffer.
     */
    const SharedPtr<CommandBuffer>& get_current_command_buffer() const {
        return m_command_buffers[m_current_frame];
    }

    bool is_recording() const { return m_is_recording; }

    /**
     * \brief get the fence for this frame.
     * \return The fence.
     */
    const VkFence& get_current_fence() const {
        return m_main_fences[m_current_frame];
    }

    SharedPtr<Mesh> get_cube_mesh() const { return m_cube_mesh; }

    const VertexBufferDescription& get_meshes_vertices_description() const {
        return m_meshes_vertices_description;
    }

    /**
     * Sets the number of threads to use for preparing instances when calling
     * draw_mesh.
     */
    void set_num_threads_for_instancing(uint32_t num_threads) {
        m_num_threads_for_instancing = num_threads;
    }

    /**
     * Sets the minimum number of instances to use multithreading during
     * draw_mesh.
     */
    void set_min_instances_for_mt(uint32_t min_instances) {
        m_min_instances_for_mt = min_instances;
    }

    uint32_t get_num_threads_for_instancing() const {
        return m_num_threads_for_instancing;
    }

    uint32_t get_min_instances_for_mt() const { return m_min_instances_for_mt; }

    const SharedPtr<Shader>& get_lighting_vertex_shader() const {
        return m_lighting_vertex_shader;
    };

    const SharedPtr<Shader>& get_lighting_fragment_shader() const {
        return m_lighting_fragment_shader;
    };

    const VertexBufferDescription&
    get_mesh_rendering_instance_vertices_description() const {
        return m_mesh_rendering_instance_vertices_description;
    }

    const SharedPtr<DescriptorSetLayout>& get_texture_array_layout() const {
        return m_texture_array_layout;
    }

    const SharedPtr<DescriptorSetLayout>&
    get_camera_uniform_set_layout() const {
        return m_camera_uniform_set_layout;
    }

    const SharedPtr<DescriptorSet>& get_current_texture_array() const {
        return m_texture_arrays[m_current_frame];
    }

    const SharedPtr<DescriptorSet>& get_current_camera_uniform_set() const {
        return m_camera_uniform_sets[m_current_frame];
    }

  private:
    void draw_meshes();
    void draw_quads();

    void create_default_textures(const SharedPtr<TextureLibrary>& texture_lib);
    void load_default_shaders(const SharedPtr<ShaderLibrary>& shader_lib);

    void create_depth_image();

    void setup_ui_quad_pipeline();
    void setup_lighting_pipeline();
    void setup_skybox_pipeline();
    void setup_camera_uniform();

    void recreate_swapchain();

    void prepare_camera_uniform();

    void load_fonts();

    void create_ui_camera();

  private:
    // Vulkan //
    const VulkanContext* m_vulkan_state;

    uint32_t m_current_frame = 0;
    uint32_t m_current_image_index = 0;
    uint32_t m_max_frames_in_flight;

    bool m_is_recording = false;

    std::vector<SharedPtr<CommandBuffer>> m_command_buffers;
    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_available_semaphores;
    std::vector<VkFence> m_main_fences;

    SharedPtr<SwapChain> m_swapchain;
    bool m_swapchain_recreated_this_frame = false;
    bool m_framebuffer_resized = false;

    SharedPtr<DescriptorPool> m_sampler_descriptor_pool;
    std::unique_ptr<TextureSampler>
        m_texture_sampler; // We should be able to use a single sampler...
    std::vector<SharedPtr<DescriptorSet>>
        m_texture_arrays; // One for each frame in flight
    std::vector<DescriptorSpec> m_texture_specs;
    SharedPtr<DescriptorSetLayout> m_texture_array_layout;

    std::unique_ptr<TextureSampler>
        m_texture_cube_sampler; // We should be able to use a single sampler...
    std::vector<SharedPtr<DescriptorSet>>
        m_texture_cube_sets; // One for each frame in flight
    SharedPtr<DescriptorSetLayout> m_texture_cube_layout;

    SharedPtr<Pipeline> m_skybox_pipeline;
    SharedPtr<Shader> m_skybox_vertex_shader;
    SharedPtr<Shader> m_skybox_fragment_shader;
    VertexBufferDescription m_skybox_vertex_buffer_description;
    SharedPtr<Mesh> m_skybox_mesh;

    uint32_t m_available_texture_index = 0;

    SharedPtr<Texture> m_white_texture;
    SharedPtr<Texture> m_black_texture;
    SharedPtr<Texture> m_gray_texture;

    SharedPtr<Pipeline> m_lighting_pipeline;
    PipelineCreateInfo m_default_lighting_pipeline_create_info;
    SharedPtr<Shader> m_lighting_vertex_shader;
    SharedPtr<Shader> m_lighting_fragment_shader;

    VertexBufferDescription m_meshes_vertices_description;

    std::unique_ptr<Image> m_depth_image;

    SharedPtr<ShaderLibrary> m_shaders;

    // Put the texture library below m_texture_specs, so the textures are
    // destroyed before m_texture_specs
    SharedPtr<TextureLibrary> m_textures;

    std::unique_ptr<Buffer> m_instance_staging_buffer;

    // Mesh Rendering Instances //
    std::vector<std::vector<MeshRenderingShaderInstanceData>>
        m_mesh_rendering_shader_instances; // One for each frame in flight
    std::vector<std::vector<MeshInstances>>
        m_mesh_rendering_instances; // One for each frame in flight
    std::vector<SharedPtr<VertexBuffer>>
        m_mesh_rendering_instances_buffers; // One for each frame in flight
    VertexBufferDescription m_mesh_rendering_instance_vertices_description;

    std::vector<std::vector<UIQuadShaderInstanceData>>
        m_ui_quad_shader_instances; // One for each frame in flight
    std::vector<SharedPtr<VertexBuffer>>
        m_ui_quad_instances_buffers; // One for each frame in flight
    VertexBufferDescription m_ui_quad_instance_vertices_description;

    SharedPtr<DescriptorPool> m_camera_uniform_pool;
    std::vector<SharedPtr<DescriptorSet>> m_camera_uniform_sets;
    SharedPtr<DescriptorSetLayout> m_camera_uniform_set_layout;
    std::vector<std::unique_ptr<UniformBuffer>>
        m_camera_uniform_buffers; // One for each frame in flight

    //  Cube  //
    SharedPtr<Mesh> m_cube_mesh;

    // Quad //
    SharedPtr<Mesh> m_ui_quad_mesh;

    std::unique_ptr<Pipeline> m_ui_quad_pipeline;
    VertexBufferDescription m_ui_quad_vertices_description;
    SharedPtr<Shader> m_ui_quad_vertex_shader;
    SharedPtr<Shader> m_ui_quad_fragment_shader;

    SharedPtr<DescriptorPool> m_ui_quad_uniform_pool;
    std::vector<std::unique_ptr<DescriptorSet>> m_ui_quad_uniform_sets;
    SharedPtr<DescriptorSetLayout> m_ui_quad_uniform_set_layout;
    std::vector<std::unique_ptr<UniformBuffer>>
        m_ui_quad_uniform_buffers; // One for each frame in flight

    // -- //
    glm::mat4 m_view_projection_matrix;
    PerspectiveCamera m_perspective_camera;

    SharedPtr<DescriptorPool> m_lights_uniform_pool;
    std::vector<std::unique_ptr<DescriptorSet>>
        m_lights_set; // One for each frame in flight
    SharedPtr<DescriptorSetLayout> m_lights_set_layout;

    std::vector<std::vector<DirectionalLight>>
        m_directional_lights; // One for each frame in flight
    std::vector<std::unique_ptr<UniformBuffer>>
        m_directional_lights_uniform_buffers;

    std::vector<std::vector<PointLight>>
        m_point_lights; // One for each frame in flight
    std::vector<std::unique_ptr<UniformBuffer>> m_point_lights_uniform_buffers;

    uint32_t m_min_instances_for_mt;
    uint32_t m_num_threads_for_instancing;

    bool m_recreate_swapchain_next_frame = false;
    bool m_vsync = true;

    bool m_shutting_down = false;

    FontLibrary m_font_library;
    SharedPtr<Font> m_selected_font = nullptr;

    glm::mat4 m_ui_projection;
};
} // namespace Helios
