#pragma once

#include <Windows.h>
#include <iostream>
#include <string_view>

class HkeyUser
{
public:
    template <class S>
    HkeyUser(const S &subkey, REGSAM desired)
    {
        ok_ = RegOpenKeyEx(HKEY_CURRENT_USER, subkey, 0, desired, &raw_) == ERROR_SUCCESS;
    }

    ~HkeyUser()
    {
        RegCloseKey(raw_);
    }

public:
    template <class T>
    T read(const T &path) const
    {
        if (!ok_)
            return T{};

        DWORD len = 0;
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, NULL, &len) != ERROR_SUCCESS)
        {
            return T{};
        }

        T value((size_t)len / sizeof(T::value_type), ' ');
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, (BYTE *)value.data(), &len) != ERROR_SUCCESS)
        {
            return T{};
        }
        return value;
    }

    template <class T>
    bool write(const T &name, const T &value)
    {
        auto len = (DWORD)(value.size() * sizeof(T::value_type));
        return ok_ && RegSetValueEx(raw_, name.data(), 0, REG_SZ, (const BYTE *)value.data(), len) == ERROR_SUCCESS;
    }

    template <class T>
    bool remove(const T &name)
    {
        return ok_ && RegDeleteValue(raw_, name.data()) == ERROR_SUCCESS;
    }

private:
    HKEY raw_{ 0 };
    bool ok_{ false };
};
