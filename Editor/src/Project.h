#pragma once
#include <filesystem>
#include <optional>
#include <string>

struct ProjectProperties {
    std::string name;
    std::optional<std::string> default_scene;
    float fixed_update_rate;
};

class Project {
  public:
    Project(const std::filesystem::path& project_path);
    Project() = default;

    const ProjectProperties& get_properties() const { return m_props; }
    const std::filesystem::path& get_project_path() const {
        return m_project_path;
    }

    bool is_valid() const { return m_valid; };

    void set_default_scene(const std::string& path);
    void set_fixed_update_rate(float rate) { m_props.fixed_update_rate = rate; }

  private:
    void new_project(std::filesystem::path project_path,
                     std::filesystem::path project_file_path);
    void load_project(std::filesystem::path project_file_path);

  private:
    ProjectProperties m_props;
    std::filesystem::path m_project_path;
    bool m_valid = true;
};
