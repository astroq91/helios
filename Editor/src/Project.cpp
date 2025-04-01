#include "Project.h"
#include "Helios/Core/Log.h"
#include "yaml-cpp/node/parse.h"
#include <filesystem>
#include <fstream>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

Project::Project(const std::filesystem::path& project_path)
    : m_project_path(project_path), m_props({
                                        .name = "Project",
                                        .default_scene = "scenes/main.scene",
                                        .fixed_update_rate = 1.0f / 50,
                                    }) {
    fs::path proj_path = project_path;

    fs::path project_file_path = proj_path / "settings.proj";

    if (fs::exists(proj_path)) {
        load_project(project_file_path);
    } else {
        new_project(proj_path, project_file_path);
    }
}

void Project::set_default_scene(const std::string& path) {
    fs::path scene_path = m_project_path / path;
    std::ifstream stream(scene_path);
    std::stringstream str_stream;
    str_stream << stream.rdbuf();

    YAML::Node data = YAML::Load(str_stream.str());
    data["default_scene"] = path;

    std::ofstream file;
    file.open(scene_path);
    file << data;
    file.close();

    m_props.default_scene = path;
}

void Project::new_project(std::filesystem::path project_path,
                          std::filesystem::path project_file_path) {
    fs::create_directory(project_path);

    fs::create_directory(project_path / "scenes/");

    std::ofstream default_scene(project_path / m_props.default_scene.value());

    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "project_name" << YAML::Value
        << project_path.filename().string();
    out << YAML::Key << "default_scene" << YAML::Value
        << m_props.default_scene.value();
    out << YAML::Key << "fixed_update_rate" << YAML::Value
        << m_props.fixed_update_rate;
    out << YAML::EndMap;

    std::ofstream file;
    file.open(project_file_path);
    file << out.c_str();
    file.close();

    m_props.name = project_path.filename().string();
}

void Project::load_project(std::filesystem::path project_file_path) {
    if (fs::exists(project_file_path)) {
        std::ifstream stream(project_file_path);
        std::stringstream str_stream;
        str_stream << stream.rdbuf();

        YAML::Node data = YAML::Load(str_stream.str());

        if (data["project_name"]) {
            m_props.name = data["project_name"].as<std::string>();
        }
        if (data["default_scene"]) {
            std::string default_scene = data["default_scene"].as<std::string>();
            auto scene_path = m_project_path / default_scene;
            if (!default_scene.empty() && std::filesystem::exists(scene_path)) {
                m_props.default_scene = default_scene;
            } else {
                HL_ERROR("Could not load default scene: {}", default_scene);
                m_props.default_scene = std::nullopt;
            }
        }
        auto fixed_update_rate = data["fixed_update_rate"];
        if (!fixed_update_rate.IsNull() && fixed_update_rate.IsScalar()) {
            m_props.fixed_update_rate = fixed_update_rate.as<float>();
        }
    } else {
        m_valid = false;
    }
}
