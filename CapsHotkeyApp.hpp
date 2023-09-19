#pragma once

#include "CapsHotkey.hpp"
#include "res/resource.h"
#include <atlbase.h>

#include <atlapp.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <shellapi.h>
#include <simple/str.hpp>
#include <simple/use_wtl.hpp>
#include <string>

static auto simulate_mouse_down()
{
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA * 3, 0);
}

static auto simulate_mouse_up()
{
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA * 3, 0);
}

enum class MenuAction : uint8_t
{
    Exit,
    Help,
    AutoRun,
};

class CapsHotkeyApp
{
public:
    CapsHotkeyApp(HICON icon, HWND hwnd)
        : icon_logo_(icon)
        , hwnd_(hwnd)
    {
        hotkey_.register_hook(VK_OEM_7, simulate_mouse_up, "Mouse up");
        hotkey_.register_hook(VK_OEM_1, simulate_mouse_down, "Mouse down");
        hotkey_.register_hook(
            VK_OEM_2, [this] { show_main_window(); }, "Show help window");
        hotkey_.register_hook(
            char2key('q'), [this] { quit_current_app(); }, "Quit CapsHotkey");
        load_hotkey_from_cfg();

        create_notification_icon();
    }

public:
    std::wstring name() const
    {
        return std::format(L"{} {}", appid_, appver_);
    }

    std::map<int, KeyHookItem> hooks() const
    {
        return hotkey_.hooks();
    }

    void quit_current_app()
    {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd_;
        nid.uID = 1;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        DestroyWindow(hwnd_);
        PostQuitMessage(0);
    }

    void show_main_window()
    {
        if (IsWindowVisible(hwnd_))
        {
            ShowWindow(hwnd_, SW_HIDE);
        }
        else
        {
            ShowWindow(hwnd_, SW_SHOW);
            SetForegroundWindow(hwnd_);
            // BringWindowToTop(hwnd_);
        }
    }

    void toggle_autorun_enabled()
    {
        CRegKeyEx key;
        if (is_auto_run())
        {
            key.Open(HKEY_CURRENT_USER, subkey_.c_str(), KEY_WRITE);
            key.DeleteValue(appid_.c_str());
        }
        else
        {
            key.Create(HKEY_CURRENT_USER, subkey_.c_str());
            key.SetStringValue(appid_.c_str(), get_app_path().c_str());
        }
    }

    void show_context_menu()
    {
        POINT pt;
        GetCursorPos(&pt);

        auto menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::Help, TEXT("Help"));
        AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::AutoRun, TEXT("Auto Run"));
        AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
        AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::Exit, TEXT("Exit"));

        CheckMenuItem(menu, (uint16_t)MenuAction::AutoRun, is_auto_run() ? MF_CHECKED : MF_UNCHECKED);

        // Track the context menu
        SetForegroundWindow(hwnd_);
        TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x - 5, pt.y - 5, 0, hwnd_, NULL);
        PostMessage(hwnd_, WM_NULL, 0, 0);
    }

private:
    void create_notification_icon()
    {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd_;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = icon_logo_;
        lstrcpy(nid.szTip, appid_.c_str());
        Shell_NotifyIcon(NIM_ADD, &nid);
    }

    bool is_auto_run() const
    {
        TCHAR value[256];
        ULONG len = 256;
        CRegKeyEx key;
        key.Open(HKEY_CURRENT_USER, subkey_.c_str(), KEY_READ);
        return key.QueryStringValue(appid_.c_str(), value, &len) == ERROR_SUCCESS && value == get_app_path();
    }

    std::wstring get_app_path() const
    {
        std::wstring path(MAX_PATH, 0);
        GetModuleFileName(NULL, path.data(), (DWORD)path.size());
        return path.c_str();
    };

    void load_hotkey_from_cfg()
    {
        static auto read_cfg = [this](auto &&text) {
            auto parts = str::split(text, "\r", false);
            for (auto &&it : parts)
            {
                hotkey_.register_hook(it);
            }
        };

        // load default config
        {
            auto res = FindResource(NULL, MAKEINTRESOURCE(IDR_CFG2), TEXT("CFG"));
            auto locked = (char *)LockResource(LoadResource(NULL, res));
            if (locked != nullptr)
            {
                std::string text(locked, SizeofResource(NULL, res));
                read_cfg(text);
            }
            FreeResource(locked);
        }

        // load user config
        {
            namespace fs = std::filesystem;
            std::ifstream in("caps.cfg");
            std::string text({ std::istreambuf_iterator<char>(in), {} });
            read_cfg(text);
        }
    }

protected:
    HICON icon_logo_{ 0 };
    HWND hwnd_;
    std::wstring appid_{ L"Capslock Hotkey" };
    std::wstring appver_{ L"v2.5" };
    std::wstring subkey_{ L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" };
    CapsHotkey hotkey_;
};
