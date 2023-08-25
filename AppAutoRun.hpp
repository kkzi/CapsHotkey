#pragma once

#include "HkeyUser.hpp"

template <class T>
static auto is_app_autorun(const T *app_name) -> bool
{
    HkeyUser ku(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), KEY_READ);
    auto value = ku.read(std::basic_string_view<T>(app_name));
    return !value.empty();
}

template <class T>
static auto set_app_autorun(const T *app_name, const T *app_path, bool enabled) -> bool
{
    HkeyUser ku(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), KEY_READ | KEY_WRITE);
    return enabled ? ku.write(std::basic_string_view<T>(app_name), std::basic_string_view<T>(app_path)) : ku.remove(std::basic_string_view<T>(app_name));
}
