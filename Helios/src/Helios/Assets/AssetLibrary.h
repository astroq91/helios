#pragma once

#include "Asset.h"
#include "Helios/Core/Core.h"
#include "Helios/Core/Log.h"

namespace Helios {
template <typename T> class AssetLibrary {
    static_assert(std::is_base_of_v<Asset, T>, "T must be a subclass of Asset");

  public:
    void add_asset(const SharedPtr<T>& asset) {
        if (!m_assets.contains(asset->get_name())) {
            m_assets[asset->get_name()] = asset;
        } else {
            HL_ERROR("Tried to add asset ({0}) with name: {1}, but that asset "
                     "already exists.",
                     typeid(T).name(), asset->get_name());
        }
    }

    SharedPtr<T> get_asset(const std::string& name) {
        const auto it = m_assets.find(name);

        if (it == m_assets.end()) {
            // HL_ERROR("No asset ({0}) with name: '{1}'", typeid(T).name(),
            // name);
            return nullptr;
        } else {
            return it->second;
        }
    }

    void clear() { m_assets.clear(); }

  private:
    std::unordered_map<std::string, SharedPtr<T>> m_assets;
};

} // namespace Helios
