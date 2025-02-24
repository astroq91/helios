#pragma once
#include <string>

namespace Helios {
struct DialogReturn {
    std::string path = "";
    std::string name = "";
};

class IOUtils {
  public:
    static DialogReturn open_file(const std::vector<std::string>& filters,
                                  const std::string& initial_directory = "");
    static DialogReturn save_file(const std::vector<std::string>& filters,
                                  const std::string& initial_directory = "");
    static DialogReturn
    select_folder(const std::string& initial_directory = "");

    static std::string resolve_path(const std::string& base_path,
                                    const std::string& relative_path);
    static std::string relative_path(const std::string& base,
                                     const std::string& target);
};
} // namespace Helios
