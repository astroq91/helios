#include "Project.h"
#include "Helios/Core/Log.h"
#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/node/parse.h"
#include <filesystem>
#include <fstream>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

Project::Project(const std::filesystem::path& project_path)
    : m_project_path(project_path), m_settings({
                                        .name = "Project",
                                        .default_scene = "scenes/main.scene",
                                        .fixed_update_rate = 1.0f / 50,
                                        .vsync = true,
                                        .instancing_settings =
                                            {
                                                .min_instances_for_mt = 100,
                                                .num_threads_for_mt = 15,
                                            },
                                    }) {
    m_project_file_path = m_project_path / "settings.proj";

    if (fs::exists(m_project_path)) {
        load_project();
    } else {
        m_settings.name = project_path.filename().string();
        new_project();
    }
}

void Project::save() {
    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "project_name" << YAML::Value << m_settings.name;
    out << YAML::Key << "default_scene" << YAML::Value;
    if (m_settings.default_scene) {
        out << m_settings.default_scene.value();
    } else {
        out << YAML::Null;
    }
    out << YAML::Key << "fixed_update_rate" << YAML::Value
        << m_settings.fixed_update_rate;
    out << YAML::Key << "vsync" << YAML::Value << m_settings.vsync;

    {
        out << YAML::Key << "instancing" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "min_instances_for_mt" << YAML::Value
            << m_settings.instancing_settings.min_instances_for_mt;
        out << YAML::Key << "num_threads_for_mt" << YAML::Value
            << m_settings.instancing_settings.num_threads_for_mt;

        out << YAML::EndMap;
    }
    out << YAML::EndMap;

    std::ofstream file;
    file.open(m_project_file_path);
    file << out.c_str();
    file.close();
}

void Project::new_project() {
    fs::create_directory(m_project_path);
    fs::create_directory(m_project_path / "scenes/");

    std::ofstream default_scene(m_project_path /
                                m_settings.default_scene.value());

    save();
}

void Project::load_project() {
    if (fs::exists(m_project_file_path)) {
        std::ifstream stream(m_project_file_path);
        std::stringstream str_stream;
        str_stream << stream.rdbuf();

        YAML::Node data = YAML::Load(str_stream.str());

        auto project_name = data["project_name"];
        if (!project_name.IsNull() && project_name.IsScalar()) {
            m_settings.name = project_name.as<std::string>();
        }

        auto default_scene = data["default_scene"];
        if (!default_scene.IsNull() && default_scene.IsScalar()) {
            std::string default_scene_str = default_scene.as<std::string>();
            auto scene_path = m_project_path / default_scene_str;
            if (!default_scene_str.empty() &&
                std::filesystem::exists(scene_path)) {
                m_settings.default_scene = default_scene_str;
            } else {
                HL_ERROR("Could not load default scene: {}", default_scene_str);
                m_settings.default_scene = std::nullopt;
            }
        } else {
            m_settings.default_scene = std::nullopt;
        }
        auto fixed_update_rate = data["fixed_update_rate"];
        if (!fixed_update_rate.IsNull() && fixed_update_rate.IsScalar()) {
            m_settings.fixed_update_rate = fixed_update_rate.as<float>();
        }

        auto vsync = data["vsync"];
        if (!vsync.IsNull() && vsync.IsScalar()) {
            m_settings.vsync = vsync.as<bool>();
        }

        auto instancing = data["instancing"];
        if (!instancing.IsNull() && instancing.IsMap()) {
            auto min_instances_for_mt = instancing["min_instances_for_mt"];
            if (!min_instances_for_mt.IsNull() &&
                min_instances_for_mt.IsScalar()) {
                m_settings.instancing_settings.min_instances_for_mt =
                    min_instances_for_mt.as<uint32_t>();
            }
            auto num_threads_for_mt = instancing["num_threads_for_mt"];
            if (!num_threads_for_mt.IsNull() && num_threads_for_mt.IsScalar()) {
                m_settings.instancing_settings.num_threads_for_mt =
                    num_threads_for_mt.as<uint32_t>();
            }
        }
    } else {
        m_valid = false;
    }
}
