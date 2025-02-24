#include "Project.h"
#include "yaml-cpp/node/parse.h"
#include <filesystem>
#include <fstream>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

Project::Project(const std::string& project_path)
    : m_project_path(project_path) {
    fs::path proj_path = fs::path(project_path);

    // Add trailing slash if missing
    if (!proj_path.empty() && proj_path.has_filename()) {
        proj_path += fs::path::preferred_separator;
    }

    fs::path project_file_path = proj_path / "project.yaml";

    if (fs::exists(proj_path)) {
        load_project(project_file_path);
    } else {
        new_project(proj_path, project_file_path);
    }
}

void Project::set_default_scene(const std::string& path) {
    fs::path scene_path = fs::path(m_project_path) / path;
    std::ifstream stream(scene_path);
    std::stringstream str_stream;
    str_stream << stream.rdbuf();

    YAML::Node data = YAML::Load(str_stream.str());
    data["default_scene"] = path;

    std::ofstream file;
    file.open(scene_path);
    file << data;
    file.close();

    m_default_scene = path;
}

void Project::new_project(std::filesystem::path project_path,
                          std::filesystem::path project_file_path) {
    fs::create_directory(project_path);

    fs::create_directory(project_path / "scenes/");
    std::ofstream default_scene(project_path / "scenes/main_scene.yaml");

    YAML::Emitter out;

    out << YAML::BeginMap;
    out << YAML::Key << "project_name" << YAML::Value
        << project_path.parent_path().filename().string();
    out << YAML::Key << "default_scene" << YAML::Value
        << "scenes/main_scene.yaml";

    out << YAML::EndMap;

    std::ofstream file;
    file.open(project_file_path);
    file << out.c_str();
    file.close();

    m_name = project_path.filename();
}

void Project::load_project(std::filesystem::path project_file_path) {
    if (fs::exists(project_file_path)) {
        std::ifstream stream(project_file_path);
        std::stringstream str_stream;
        str_stream << stream.rdbuf();

        YAML::Node data = YAML::Load(str_stream.str());

        if (data["project_name"]) {
            m_name = data["project_name"].as<std::string>();
        }
        if (data["default_scene"]) {
            std::string default_scene = data["default_scene"].as<std::string>();
            if (!default_scene.empty()) {
                m_default_scene = default_scene;
            }
        }
    } else {
        m_valid = false;
    }
}
