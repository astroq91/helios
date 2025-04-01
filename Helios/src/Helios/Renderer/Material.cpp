#include "Material.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include "Helios/Renderer/Shader.h"
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Helios {
bool Material::init(const std::filesystem::path& path) {
    std::ifstream stream(
        IOUtils::resolve_path(Application::get().get_asset_base_path(), path)
            .string());

    if (stream.fail()) {
        HL_ERROR("Failed to load material {0}", path.string());
        return false;
    }

    std::stringstream str_stream;
    str_stream << stream.rdbuf();

    try {

        YAML::Node data = YAML::Load(str_stream.str());

        if (data["diffuse"]) {
            auto diffuse = data["diffuse"];
            if (diffuse["image"]) {
                auto image = diffuse["image"];
                if (!image.IsNull() && image.IsScalar()) {
                    auto relative_path = image.as<std::string>();
                    if (!relative_path.empty()) {
                        auto texture =
                            Application::get().get_asset_manager().get_texture(
                                relative_path);
                        if (texture == nullptr) {
                            m_diffuse = Texture::create(relative_path);
                            Application::get().get_asset_manager().add_texture(
                                m_diffuse);
                        } else {
                            m_diffuse = texture;
                        }
                    }
                }
            }
        }

        if (data["specular"]) {
            auto specular = data["specular"];
            if (specular["image"]) {
                auto image = specular["image"];
                if (!image.IsNull() && image.IsScalar()) {
                    auto relative_path = image.as<std::string>();
                    if (!relative_path.empty()) {
                        auto texture =
                            Application::get().get_asset_manager().get_texture(
                                relative_path);
                        if (texture == nullptr) {
                            m_specular = Texture::create(relative_path);
                            Application::get().get_asset_manager().add_texture(
                                m_specular);
                        } else {
                            m_specular = texture;
                        }
                    }
                }
            }
        }
        if (data["emission"]) {
            auto emission = data["emission"];
            if (emission["image"]) {
                auto image = emission["image"];
                if (!image.IsNull() && image.IsScalar()) {
                    auto relative_path = image.as<std::string>();
                    if (!relative_path.empty()) {
                        auto texture =
                            Application::get().get_asset_manager().get_texture(
                                relative_path);
                        if (texture == nullptr) {
                            m_emission = Texture::create(relative_path);
                            Application::get().get_asset_manager().add_texture(
                                m_emission);
                        } else {
                            m_emission = texture;
                        }
                    }
                }
            }
        }
        auto shininess = data["shininess"];
        if (shininess && !shininess.IsNull() && shininess.IsScalar()) {
            m_shininess = shininess.as<float>();
        } else {
            // TODO: Standardize this. This number is kind of all over the place
            m_shininess = 32;
        }

        if (data["shaders"] && data["shaders"].IsMap()) {
            auto shaders = data["shaders"];
            if (shaders["vertex_shader"] &&
                shaders["vertex_shader"].IsScalar()) {
                auto vs = shaders["vertex_shader"];
                auto relative_path = vs.as<std::string>();
                if (!relative_path.empty()) {
                    auto shader =
                        Application::get().get_asset_manager().get_shader(
                            relative_path);
                    if (shader == nullptr) {
                        m_vertex_shader =
                            Shader::create(relative_path, relative_path);
                        if (m_vertex_shader) {
                            Application::get().get_asset_manager().add_shader(
                                m_vertex_shader);
                        }
                    } else {
                        m_vertex_shader = shader;
                    }
                }
            }
            if (shaders["fragment_shader"] &&
                shaders["fragment_shader"].IsScalar()) {
                auto fs = shaders["fragment_shader"];
                auto relative_path = fs.as<std::string>();
                if (!relative_path.empty()) {
                    auto shader =
                        Application::get().get_asset_manager().get_shader(
                            relative_path);
                    if (shader == nullptr) {
                        m_fragment_shader =
                            Shader::create(relative_path, relative_path);
                        if (m_fragment_shader) {
                            Application::get().get_asset_manager().add_shader(
                                m_fragment_shader);
                        }
                    } else {
                        m_fragment_shader = shader;
                    }
                }
            }
        }

    } catch (YAML::Exception& e) {
        HL_ERROR("Failed to parse material file: {}", e.what());
        return false;
    }
    return true;
}
} // namespace Helios
