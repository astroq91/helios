#include "Material.h"
#include "Helios/Core/Application.h"
#include "Helios/Core/IOUtils.h"
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Helios {
void Material::init(const std::string& path) {
    std::ifstream stream(
        IOUtils::resolve_path(Application::get().get_asset_base_path(), path));
    std::stringstream str_stream;
    str_stream << stream.rdbuf();

    try {

        YAML::Node data = YAML::Load(str_stream.str());
        auto diffuse = data["diffuse"];
        if (diffuse) {
            auto image = diffuse["image"];
            if (image) {
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

        auto specular = data["specular"];
        if (specular) {
            auto image = specular["image"];
            if (image) {
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
        auto emission = data["emission"];
        if (emission) {
            auto image = emission["image"];
            if (image) {
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
        auto shininess = data["shininess"];
        if (shininess) {
            m_shininess = shininess.as<float>();
        } else {
            // TODO: Standardize this. This number is kind of all over the place
            m_shininess = 32;
        }

    } catch (YAML::Exception& e) {
        HL_ERROR("Failed to parse material file: {}", e.what());
    }
}
} // namespace Helios
