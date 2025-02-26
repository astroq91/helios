#include "ImGuiLayer.h"

#include "Helios/Core/Application.h"
#include "Helios/Renderer/Renderer.h"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"

namespace Helios {
static ImGui_ImplVulkanH_Window g_main_window_data;
static uint32_t g_min_image_count = 2;

PFN_vkVoidFunction volk_loader(const char* function_name, void* user_data) {
    return vkGetInstanceProcAddr(volkGetLoadedInstance(), function_name);
}

void ImGuiLayer::on_attach() {
    Application& app = Application::get();
    GLFWwindow* window = app.get_native_window();

    const VulkanContext& context = app.get_vulkan_manager()->get_context();
    Renderer& renderer = app.get_renderer();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable
    // Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Example descriptor counts for ImGui
    constexpr uint32_t max_imgui_descriptors = 1000; // Adjust as needed

    // Descriptor pool sizes
    std::vector<VkDescriptorPoolSize> pool_sizes = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, max_imgui_descriptors},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, max_imgui_descriptors},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, max_imgui_descriptors},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_imgui_descriptors},
    };

    // create descriptor pool
    m_descriptor_pool = DescriptorPool::CreateUnique(
        max_imgui_descriptors, pool_sizes,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    VkPipelineRenderingCreateInfoKHR dynamic_info{};
    dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    dynamic_info.pNext = VK_NULL_HANDLE;
    dynamic_info.colorAttachmentCount = 1;
    dynamic_info.pColorAttachmentFormats =
        &renderer.get_swapchain()->get_vk_format();
    dynamic_info.depthAttachmentFormat =
        VulkanUtils::find_depth_format(context.physical_device);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = context.instance;
    init_info.Device = context.device;
    init_info.PhysicalDevice = context.physical_device;
    init_info.DescriptorPool = m_descriptor_pool->get_vk_pool();
    init_info.QueueFamily = VulkanUtils::find_queue_families(
                                context.physical_device, context.surface)
                                .graphics_family.value();
    init_info.Queue = context.graphics_queue;
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo = dynamic_info;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.MinImageCount = g_min_image_count;
    init_info.ImageCount = renderer.get_swapchain()->get_image_count();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = [](VkResult error) {
        if (error != VK_SUCCESS) {
            HL_ERROR("ImGui_ImplVulkan_Init error");
        }
    };

    ImGui_ImplVulkan_LoadFunctions(volk_loader, nullptr);
    ImGui_ImplVulkan_Init(&init_info);

    // create the textures
    ImGui_ImplVulkan_CreateFontsTexture();

    // ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiLayer::on_detach() {}

void ImGuiLayer::on_event(Event& e) {
    if (m_block_events) {
        ImGuiIO& io = ImGui::GetIO();
        e.handled |= e.is_in_category(EventCategoryMouse) & io.WantCaptureMouse;
        e.handled |=
            e.is_in_category(EventCategoryKeyboard) & io.WantCaptureKeyboard;
    }
}

ImGuiLayer::~ImGuiLayer() {
    Application& app = Application::get();
    const VulkanContext& context = app.get_vulkan_manager()->get_context();

    ImGui_ImplVulkanH_DestroyWindow(context.instance, context.device,
                                    &g_main_window_data, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::begin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end() {
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::get();
    io.DisplaySize = ImVec2((float)app.get_window().get_width(),
                            (float)app.get_window().get_height());

    Renderer& renderer = app.get_renderer();

    ImGui::EndFrame();

    if (renderer.swapchain_recreated_this_frame()) {
        return;
    }

    // Rendering
    ImGui::Render();

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    renderer.begin_rendering({
        .color_image_layout = VK_IMAGE_LAYOUT_GENERAL,
        .color_load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .color_store_op = VK_ATTACHMENT_STORE_OP_STORE,
    });

    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        renderer.get_current_command_buffer()->get_command_buffer());

    renderer.end_rendering();
}
} // namespace Helios
