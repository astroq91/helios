#pragma once
#include <string>
#include <filesystem>

namespace Helios {
struct DialogReturn {
    std::filesystem::path path = "";
    std::filesystem::path name = "";
};

struct FilterEntry {
    std::string name;
    std::string filter;
};

class IOUtils {
  public:
    static DialogReturn open_file(const std::vector<FilterEntry>& filters,
                                    const std::filesystem::path& initial_directory = std::filesystem::path());
    static DialogReturn
    save_file(const std::vector<FilterEntry>& filters,
              const std::filesystem::path& initial_directory =
                            std::filesystem::path());
    static DialogReturn
    select_folder(const std::filesystem::path& initial_directory = "");

    static std::filesystem::path resolve_path(const std::filesystem::path & base_path, const std::filesystem::path & relative_path);
    static std::filesystem::path relative_path(const std::filesystem::path & base, const std::filesystem::path & target);
};
} // namespace Helios
