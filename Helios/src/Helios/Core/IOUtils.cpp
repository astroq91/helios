#include "IOUtils.h"
#include <filesystem>
#include <tinyfiledialogs.h>

#ifdef _WINDOWS
#include <commdlg.h>
#endif

namespace {
const char* const* vector_to_char_arrray(const std::vector<std::string>& vec) {
    // Allocate an array of pointers to the C-strings
    const char** cstrings = new const char*[vec.size()];
    for (size_t i = 0; i < vec.size(); ++i) {
        cstrings[i] = vec[i].c_str();
    }
    return cstrings;
}
} // namespace

namespace Helios {
/**
 * \brief Open a file dialog to select a file.
 * \return A struct containing return data.
 */

DialogReturn IOUtils::open_file(const std::vector<std::string>& filters,
                                const std::string& initial_directory) {

    std::string init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = BASE_DIR;
    }
#endif

    DialogReturn dialog_return{};

    auto filter_array = vector_to_char_arrray(filters);

    const char* file = tinyfd_openFileDialog("Open file", init_dir.c_str(),
                                             static_cast<int>(filters.size()),
                                             filter_array, nullptr, 0);

    if (file != nullptr) {
        dialog_return.path = file;
        dialog_return.name = std::filesystem::path(file).filename().string();
    }
    delete[] filter_array;

    return dialog_return;
}

DialogReturn IOUtils::save_file(const std::vector<std::string>& filters,
                                const std::string& initial_directory) {

    std::string init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = BASE_DIR;
    }
#endif

    DialogReturn dialog_return;

    auto filter_array = vector_to_char_arrray(filters);

    const char* file = tinyfd_saveFileDialog("Save file", init_dir.c_str(),
                                             static_cast<int>(filters.size()),
                                             filter_array, nullptr);

    if (file != nullptr) {
        dialog_return.path = file;
        dialog_return.name = std::filesystem::path(file).filename().string();
    }
    delete[] filter_array;

    return dialog_return;
}

DialogReturn IOUtils::select_folder(const std::string& initial_directory) {
    std::string init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = BASE_DIR;
    }
#endif

    DialogReturn ret;
    const char* folder =
        tinyfd_selectFolderDialog("Select folder", init_dir.c_str());

    if (folder != nullptr) {
        ret.path = folder;
    }
    return ret;
}

std::string IOUtils::resolve_path(const std::string& base_path,
                                  const std::string& relative_path) {
    if (std::filesystem::path(relative_path).is_absolute()) {
        return relative_path; // Already absolute
    }
    return std::filesystem::absolute(std::filesystem::path(base_path) /
                                     relative_path)
        .string();
}

std::string IOUtils::relative_path(const std::string& base,
                                   const std::string& target) {
    std::filesystem::path base_path = std::filesystem::canonical(base);
    std::filesystem::path target_path = std::filesystem::canonical(target);

    std::filesystem::path relative_path =
        std::filesystem::relative(target_path, base_path);

    return relative_path.string();
}
} // namespace Helios
