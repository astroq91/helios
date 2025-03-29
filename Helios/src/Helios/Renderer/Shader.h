#pragma once
#include <filesystem>
#include <map>
#include <string>

#include "Helios/Core/Core.h"

#include <glm/glm.hpp>
#include <volk/volk.h>

#include "Helios/Assets/Asset.h"

namespace Helios {
class Shader : public Asset {
  public:
    /**
     * \brief create a new Shader.
     * \param name The name of the shader.
     * \param path The filepath to the shader.
     */
    static SharedPtr<Shader> create(const std::string& name,
                              const std::filesystem::path& path) {
        SharedPtr<Shader> sh = SharedPtr<Shader>::create();
        sh->init_asset(name);
        sh->init(path);
        return sh;
    }

    VkShaderModule get_vk_module() const { return m_module; }

    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

  private:
    void init(const std::filesystem::path& path);

  private:
    VkShaderModule m_module;

    bool m_is_initialized = false;
};
} // namespace Helios
