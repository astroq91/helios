#pragma once
#include <filesystem>
#include <optional>
#include <string>

class Project {
  public:
    Project(const std::string& project_path);
    Project() = default;

    const std::string& get_name() const { return m_name; }
    const std::string& get_project_path() const { return m_project_path; }

    bool is_valid() const { return m_valid; };
    std::optional<std::string> get_default_scene() const {
        return m_default_scene;
    };

    void set_default_scene(const std::string& path);

  private:
    void new_project(std::filesystem::path project_path,
                     std::filesystem::path project_file_path);
    void load_project(std::filesystem::path project_file_path);

  private:
    std::string m_name;
    std::string m_project_path;
    std::optional<std::string> m_default_scene;
    bool m_valid = true;
};
