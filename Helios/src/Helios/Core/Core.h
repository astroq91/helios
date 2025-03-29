#pragma once
#include "SharedPtr.h"
#include <filesystem>
#include <memory>

#define HL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define BIT(x) (1 << x)

#ifdef _WINDOWS
#define CONV_PATH(path) path.make_preferred()
#elif _LINUX
#define CONV_PATH(path) path
#endif