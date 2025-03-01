#include "IOUtils.h"
#include <filesystem>
#include <tinyfiledialogs.h>

#ifdef _WINDOWS
#include <commdlg.h>
#include <ranges>
#include <shobjidl.h>
#include <windows.h>
#endif

namespace {
#ifdef _LINUX
std::unique_ptr<const char*[]>
convert_filters(const std::vector<Helios::FilterEntry>& vec) {
    // Allocate an array of pointers to the C-strings
    std::unique_ptr<const char*[]> filters =
        std::make_unique<const char*[]>(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        filters[i] = vec[i].filter.c_str();
    }
    return filters;
}
#elif _WINDOWS
std::unique_ptr<char[]>
convert_filters(const std::vector<Helios::FilterEntry>& vec) {
    size_t required_size = 0;
    for (Helios::FilterEntry entry : vec) {
        required_size += entry.filter.size() + entry.name.size();
    }
    // For good measure
    required_size *= 2;

    std::unique_ptr<char[]> filters = std::make_unique<char[]>(required_size);
    uint32_t offset = 0;
    for (size_t i = 0; i < vec.size(); i++) {
        std::strcpy(&filters[offset], vec[i].name.c_str());
        offset += vec[i].name.size() + 1;

        std::strcpy(&filters[offset], vec[i].filter.c_str());
        offset += vec[i].filter.size() + 1;
    }
    return filters;
}
#endif

} // namespace

namespace Helios {
/**
 * \brief Open a file dialog to select a file.
 * \return A struct containing return data.
 */

DialogReturn
IOUtils::open_file(const std::vector<FilterEntry>& filters,
                   const std::filesystem::path& initial_directory) {
    DialogReturn dialog_return{};
    std::filesystem::path init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = std::filesystem::path(BASE_DIR);
    }
#endif

    init_dir = init_dir.make_preferred();

    auto filter_array = convert_filters(filters);

#ifdef _LINUX
    const char* file = tinyfd_openFileDialog(
        "Open file", init_dir.string().c_str(),
        static_cast<int>(filters.size()), filter_array.get(), nullptr, 0);

    if (file != nullptr) {
        dialog_return.path = std::filesystem::path(file);
        dialog_return.name = std::filesystem::path(file).filename().string();
    }
#elif _WINDOWS

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char file[260] = {0};

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = filter_array.get();
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir =
        (init_dir.native() | std::ranges::to<std::string>()).c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        dialog_return.path = std::filesystem::path(file);
        dialog_return.name = std::filesystem::path(file).filename().string();
    }

#endif

    return dialog_return;
}

DialogReturn
IOUtils::save_file(const std::vector<FilterEntry>& filters,
                   const std::filesystem::path& initial_directory) {

    DialogReturn dialog_return{};
    std::filesystem::path init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = std::filesystem::path(BASE_DIR);
    }
#endif

    init_dir = init_dir.make_preferred();

    auto filter_array = convert_filters(filters);

#ifdef _LINUX

    const char* file = tinyfd_saveFileDialog(
        "Save file", init_dir.string().c_str(),
        static_cast<int>(filters.size()), filter_array.get(), nullptr);

    if (file != nullptr) {
        dialog_return.path = std::filesystem::path(file);
        dialog_return.name = std::filesystem::path(file).filename().string();
    }

#elif _WINDOWS

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char file[260] = {0};

    // Initialize OPENFILENAME
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrFilter = filter_array.get();
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir =
        (init_dir.native() | std::ranges::to<std::string>()).c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetSaveFileName(&ofn) == TRUE) {
        dialog_return.path = std::filesystem::path(file);
        dialog_return.name = std::filesystem::path(file).filename().string();
    }

#endif

    return dialog_return;
}

DialogReturn
IOUtils::select_folder(const std::filesystem::path& initial_directory) {
    DialogReturn dialog_return{};
    std::filesystem::path init_dir = initial_directory;

#ifdef _DEBUG
    if (init_dir.empty()) {
        init_dir = std::filesystem::path(BASE_DIR);
    }
#endif

    init_dir = init_dir.make_preferred();

#ifdef _LINUX
    const char* folder =
        tinyfd_selectFolderDialog("Select folder", init_dir.string().c_str());

    if (folder != nullptr) {
        dialog_return.path = std::filesystem::path(folder);
    }
#elif _WINDOWS
    HRESULT hr =
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // Initialize COM

    if (SUCCEEDED(hr)) {
        IFileDialog* pFileDialog;
        hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                              IID_IFileDialog,
                              reinterpret_cast<void**>(&pFileDialog));

        if (SUCCEEDED(hr)) {
            DWORD options;
            pFileDialog->GetOptions(&options);
            pFileDialog->SetOptions(options |
                                    FOS_PICKFOLDERS); // Set folder mode

            IShellItem* pFolder;
            hr = SHCreateItemFromParsingName(
                init_dir.wstring().c_str(), nullptr, IID_IShellItem,
                reinterpret_cast<void**>(&pFolder));

            if (SUCCEEDED(hr)) {
                // Use SetDefaultFolder instead of SetFolder
                pFileDialog->SetDefaultFolder(pFolder);
                pFolder->Release();
            } else {
                // Handle error, e.g., log or display a message
                std::cerr << "Failed to create IShellItem from path: "
                          << init_dir << std::endl;
            }

            if (SUCCEEDED(pFileDialog->Show(nullptr))) {
                IShellItem* pItem;
                if (SUCCEEDED(pFileDialog->GetResult(&pItem))) {
                    PWSTR pszFilePath;
                    if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH,
                                                        &pszFilePath))) {
                        dialog_return.path = std::filesystem::path(
                            std::wstring(pszFilePath, wcslen(pszFilePath)));
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileDialog->Release();
        }
        CoUninitialize(); // Clean up COM
    }

#endif

    return dialog_return;
}

std::filesystem::path
IOUtils::resolve_path(const std::filesystem::path& base_path,
                      const std::filesystem::path& relative_path) {
    if (relative_path.is_absolute()) {
        return relative_path; // Already absolute
    }
    return std::filesystem::absolute(std::filesystem::path(base_path) /
                                     relative_path)
        .string();
}

std::filesystem::path
IOUtils::relative_path(const std::filesystem::path& base,
                       const std::filesystem::path& target) {
    std::filesystem::path base_path = std::filesystem::canonical(base);
    std::filesystem::path target_path = std::filesystem::canonical(target);

    std::filesystem::path relative_path =
        std::filesystem::relative(target_path, base_path);

    std::string new_path = relative_path.string();

    // Replace backslashes with forward slashes
    std::replace(new_path.begin(), new_path.end(), '\\', '/');

    return new_path;
}

} // namespace Helios
