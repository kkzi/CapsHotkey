#pragma once

#include <Windows.h>
#include <iostream>
#include <string_view>

class HkeyUser
{
public:
    HkeyUser(std::wstring_view subkey, REGSAM desired)
    {
        ok_ = RegOpenKeyEx(HKEY_CURRENT_USER, subkey.data(), 0, desired, &raw_) == ERROR_SUCCESS;
    }

    ~HkeyUser()
    {
        RegCloseKey(raw_);
    }

public:
    std::wstring read(std::wstring_view path) const
    {
        if (!ok_)
            return TEXT("");

        DWORD len = 0;
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, NULL, &len) != ERROR_SUCCESS)
        {
            return L"";
        }

        std::wstring value(len / sizeof(char), L'\0');
        if (RegQueryValueEx(raw_, path.data(), NULL, NULL, (BYTE *)value.data(), &len) != ERROR_SUCCESS)
        {
            return L"";
        }
        return value;
    }

    bool write(std::wstring_view name, std::wstring_view value)
    {
        return ok_ && RegSetValueEx(raw_, name.data(), 0, REG_SZ, (const BYTE *)value.data(), (DWORD)value.size()) == ERROR_SUCCESS;
    }

    bool remove(std::wstring_view name)
    {
        return ok_ && RegDeleteValue(raw_, name.data()) == ERROR_SUCCESS;
    }

private:
    HKEY raw_{ 0 };
    bool ok_{ false };
};

static auto is_app_autorun(std::wstring_view app_name) -> bool
{
    HkeyUser ku(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", KEY_READ);
    auto value = ku.read(app_name);
    return !value.empty();
}

static auto set_app_autorun(std::wstring_view app_name, std::wstring_view app_path, bool enabled) -> bool
{
    HkeyUser ku(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", KEY_READ | KEY_WRITE);
    return enabled ? ku.write(app_name, app_path) : ku.remove(app_name);
}
