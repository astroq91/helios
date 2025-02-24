#include "Shader.h"
#include <fstream>

#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"

namespace Helios {
Shader::~Shader() {
    if (m_is_initialized) {
        auto module = m_module;
        auto device =
            Application::get().get_vulkan_manager()->get_context().device;

        // Enqueue the destruction command
        Application::get().get_vulkan_manager()->enqueue_for_destruction(
            [=]() { vkDestroyShaderModule(device, module, nullptr); });
    }
}

void Shader::init(const std::string& path) {
    m_is_initialized = true;

    const VulkanContext& context =
        Application::get().get_vulkan_manager()->get_context();

    std::ifstream file(
        IOUtils::resolve_path(Application::get().get_asset_base_path(), path),
        std::ios::ate | std::ios::binary);

    if (file.fail()) {
        HL_ERROR("Could not open file: {0}", path);
    }

    // Vertex //
    size_t vertexFileSize = (size_t)file.tellg();
    std::vector<char> src(vertexFileSize);

    file.seekg(0);
    file.read(src.data(), vertexFileSize);

    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = src.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(src.data());

    if (vkCreateShaderModule(context.device, &createInfo, nullptr, &m_module) !=
        VK_SUCCESS) {
        HL_ERROR("Failed to create shader module!");
    }
}
} // namespace Helios
