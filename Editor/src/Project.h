#pragma once
#include <filesystem>
#include <optional>
#include <string>

struct InstancingSettings {
    uint32_t min_instances_for_mt;
    uint32_t num_threads_for_mt;
};

struct ProjectSettings {
    std::string name;
    std::optional<std::string> default_scene;
    float fixed_update_rate;
    bool vsync;
    InstancingSettings instancing_settings;
};

class Project {
  public:
    Project(const std::filesystem::path& project_path);
    Project() = default;

    const ProjectSettings& get_settings() const { return m_settings; }
    const std::filesystem::path& get_project_path() const {
        return m_project_path;
    }

    void save();

    bool is_valid() const { return m_valid; };

    void set_default_scene(const std::optional<std::string>& path) {
        m_settings.default_scene = path;
    };
    void set_fixed_update_rate(float rate) {
        m_settings.fixed_update_rate = rate;
    }
    void set_vsync(bool vsync) { m_settings.vsync = vsync; }
    void set_instancing_settings(const InstancingSettings& new_settings) {
        m_settings.instancing_settings = new_settings;
    }

  private:
    void new_project();
    void load_project();

  private:
    ProjectSettings m_settings;
    std::filesystem::path m_project_path;
    std::filesystem::path m_project_file_path;
    bool m_valid = true;
};
