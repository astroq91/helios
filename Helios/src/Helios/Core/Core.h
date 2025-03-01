#pragma once
#include <filesystem>
#include <memory>

#define HL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define BIT(x) (1 << x)

#ifdef _WINDOWS
#define CONV_PATH(path) path.make_preferred()
#elif _LINUX
#define CONV_PATH(path) path
#endif

namespace Helios {
template <class T> using Ref = std::shared_ptr<T>;

template <class T> using Unique = std::unique_ptr<T>;

template <class T, class... Args> Ref<T> make_ref(Args... args) {
    return std::make_shared<T>(args...);
}

template <class T, class... Args> Unique<T> make_unique(Args... args) {
    return std::make_unique<T>(args...);
}
} // namespace Helios
