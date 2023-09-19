#pragma once

#include <Windows.h>
#include <iostream>
#include <string_view>

class HkeyUser
{
public:
    template <class T>
    HkeyUser(const T *subkey, REGSAM desired)
    {
        ok_ = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, desired, &raw_) == ERROR_SUCCESS;
    }

    ~HkeyUser()
    {
        RegCloseKey(raw_);
    }

public:
    template <class T>
    std::basic_string<T> read(std::basic_string_view<T> path) const
    {
        if (!ok_)
            return TEXT("");

        DWORD len = 0;
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, NULL, &len) != ERROR_SUCCESS)
        {
            return TEXT("");
        }

        std::basic_string<T> value(len / sizeof(T), '\0');
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, (BYTE *)value.data(), &len) != ERROR_SUCCESS)
        {
            return TEXT("");
        }
        return value;
    }

    template <class T>
    bool write(std::basic_string_view<T> name, std::basic_string_view<T> value)
    {
        auto len = (DWORD)(value.size() * sizeof(T));
        return ok_ && RegSetValueEx(raw_, name.data(), 0, REG_SZ, (const BYTE *)value.data(), len) == ERROR_SUCCESS;
    }

    template <class T>
    bool remove(std::basic_string_view<T> name)
    {
        return ok_ && RegDeleteValue(raw_, name.data()) == ERROR_SUCCESS;
    }

private:
    HKEY raw_{ 0 };
    bool ok_{ false };
};
