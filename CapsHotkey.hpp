#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <windows.h>

using namespace std::chrono;
using namespace std::chrono_literals;
using KeyHookFunc = std::function<void(void)>;

static constexpr uint8_t KEY_CAPSLOCK = VK_CAPITAL;  // the modifier key used
static constexpr uint8_t KEY_CTRL{ 1 };
static constexpr uint8_t KEY_ALT{ 2 };
static constexpr uint8_t KEY_SHIFT{ 4 };
static constexpr uint8_t KEY_WIN{ 8 };
static constexpr duration PRESS_TIMEOUT{ 300ms };

static bool capslock_busy_{ false };
static std::optional<time_point<system_clock>> capslock_down_time_{ std::nullopt };

struct KeyHookItem
{
    std::string desc;
    int src_keycode{ 0 };
    uint8_t tar_modified{ 0 };
    std::vector<int> targets;
    std::string func;
};

// static std::ofstream logfile_("log.txt", std::ios::trunc);
template <class... Args>
static void log(std::string_view &&fmt, Args &&...args)
{
    // std::cout << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
    // logfile_ << std::vformat(fmt, std::make_format_args(args...)) << std::endl;
    // logfile_.flush();
}

// a function from ascii to key codes
static auto char2key(char chr) -> int
{
    return (unsigned int)0x41 + std::tolower(chr) - 'a';
}

static auto key2str(UCHAR vk) -> std::string
{
    UINT scancode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    CHAR szName[128];
    int result = 0;
    switch (vk)
    {
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_RCONTROL:
    case VK_RMENU:
    case VK_LWIN:
    case VK_RWIN:
    case VK_APPS:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_END:
    case VK_HOME:
    case VK_INSERT:
    case VK_DELETE:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        scancode |= KF_EXTENDED;
    default:
        result = GetKeyNameTextA(scancode << 16, szName, 128);
    }
    if (result == 0)
        throw std::system_error(std::error_code(GetLastError(), std::system_category()), "WinAPI Error occured.");
    return szName;
}

static auto modified2longstr(int modified) -> std::string
{
    std::string prefix;
    prefix += modified & KEY_CTRL ? "Ctrl " : "";
    prefix += modified & KEY_ALT ? "Alt " : "";
    prefix += modified & KEY_SHIFT ? "Shift " : "";
    return prefix;
}

static auto key_text(int modified, int keycode) -> std::string
{
    return modified2longstr(modified) + key2str(keycode);
}

static auto keyid(uint8_t modified, int keycode) -> std::string
{
    return key_text(modified, keycode);
}

static auto is_key_pressed(int key) -> bool
{
    return GetKeyState(key) & 0x8000;
}

static auto is_specified_key(int key) -> bool
{
    static std::vector<int> specified_keys{
        VK_CAPITAL,
        VK_SHIFT,
        VK_LSHIFT,
        VK_RSHIFT,
        VK_CONTROL,
        VK_MENU,
        VK_LMENU,
        VK_RMENU,
        VK_LWIN,
        VK_RWIN,
        VK_ESCAPE,
    };
    return std::find(specified_keys.begin(), specified_keys.end(), key) != specified_keys.end();
}

static auto key_up(int key)
{
    // log("{:#x} key up", key);
    keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
}

static auto key_down(int key)
{
    // log("{:#x} key down", key);
    static constexpr int KEYEVENTF_KEYDOWN = 0;  // why doesn't that already exist???
    keybd_event(key, 0, KEYEVENTF_KEYDOWN, 0);
}

static auto key_click(auto key)
{
    key_down(key);
    key_up(key);
}

static auto key_click(uint8_t modified, std::vector<int> keys, const KeyHookFunc &func = nullptr)
{
    if (modified & KEY_CTRL)
        key_down(VK_CONTROL);
    if (modified & KEY_ALT)
        key_down(VK_MENU);
    if (modified & KEY_SHIFT)
        key_down(VK_SHIFT);
    if (modified & KEY_WIN)
        key_down(VK_LWIN);

    for (auto &&key : keys)
    {
        key_click(key);
    }
    if (func)
    {
        func();
    }

    if (modified & KEY_CTRL)
        key_up(VK_CONTROL);
    if (modified & KEY_ALT)
        key_up(VK_MENU);
    if (modified & KEY_SHIFT)
        key_up(VK_SHIFT);
    if (modified & KEY_WIN)
        key_up(VK_LWIN);
}

static std::map<std::string, KeyHookFunc> name2func_{
    {
        "delete_left_word",
        [] {
            key_click(KEY_CTRL | KEY_SHIFT, { VK_LEFT });
            key_click(KEY_CTRL, { char2key('x') });
        },
    },
    {
        "delete_to_line_begin",
        [] {
            key_click(KEY_SHIFT, { VK_HOME });
            key_click(VK_CONTROL, { char2key('x') });
        },
    },
    {
        "delete_to_line_end",
        [] {
            key_click(KEY_SHIFT, { VK_END });
            key_click(VK_CONTROL, { char2key('x') });
        },
    },
    {
        "delete_current_line",
        [] {
            key_click(VK_HOME);
            key_click(KEY_SHIFT, { VK_END });
            key_click(VK_CONTROL, { char2key('x') });
        },
    },
    {
        "new_line_up",
        [] {
            key_up(VK_SHIFT);
            key_click(VK_HOME);
            key_click(VK_RETURN);
        },
    },

};

static std::vector<KeyHookItem> key_hooks_{
    { "", char2key('h'), 0, { VK_LEFT } },
    { "", char2key('j'), 0, { VK_DOWN } },
    { "", char2key('k'), 0, { VK_UP } },
    { "", char2key('l'), 0, { VK_RIGHT } },

    { "", char2key('a'), 0, { VK_HOME } },
    { "", char2key('e'), 0, { VK_END } },

    { "Delete word backforward", char2key('w'), 0, {}, "delete_left_word" },
    { "Delete from cursor to line begin", char2key('u'), 0, {}, "delete_to_line_begin" },
    { "Delete from cursor to line end", char2key('c'), 0, {}, "delete_to_line_end" },
    { "Delete current line", char2key('s'), 0, {}, "delete_current_line" },

    { "Insert new blank line after current", char2key('o'), 0, { VK_END, VK_RETURN } },
    { "Insert new blank line before current", char2key('o'), 0, {}, "new_line_up" },

    { "Copy", char2key('y'), KEY_CTRL, { VK_INSERT } },
    { "Paste", char2key('p'), KEY_SHIFT, { VK_INSERT } },

    { "Quit", char2key('q'), 0, {}, "quit_current_app" },
    { "Help", VK_OEM_2 /* ? */, 0, {}, "show_help_window" },
};

static std::map<int, KeyHookItem> key2hook_ = ([] {
    std::map<int, KeyHookItem> key2hook;
    for (auto it : key_hooks_)
    {
        key2hook[it.src_keycode] = it;
    }
    return key2hook;
})();

static auto process_keydown(int keycode) -> bool
{
    if (keycode == KEY_CAPSLOCK)
    {
        if (!capslock_down_time_)
        {
            capslock_down_time_ = system_clock::now();
            return true;
        }
        else if (!capslock_busy_)
        {
            return true;
        }
    }
    else if (capslock_down_time_)
    {
        if (key2hook_.contains(keycode))
        {
            const auto &item = key2hook_.at(keycode);
            const auto &func = name2func_.contains(item.func) ? name2func_.at(item.func) : nullptr;
            key_click(item.tar_modified, item.targets, func);
            return true;
        }
    }
    return false;
}

static auto process_keyup(int keycode) -> bool
{
    if (auto capslock_pressed = capslock_down_time_.has_value(); !capslock_pressed)
    {
        return false;
    }
    if (keycode == KEY_CAPSLOCK)
    {
        auto duration = duration_cast<milliseconds>(system_clock::now() - capslock_down_time_.value());
        if (duration <= PRESS_TIMEOUT && !capslock_busy_)
        {
            capslock_busy_ = true;
            key_click(KEY_CAPSLOCK);
            capslock_busy_ = false;
        }
        capslock_down_time_ = std::nullopt;
    }
    // else if (keycode == VK_LSHIFT || keycode == VK_LMENU)  // ignore input method change
    //{
    //    key_up(keycode);
    //    return true;
    //}
    return false;
}

static auto process_hook_keyboard(int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (nCode != HC_ACTION)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    auto keycode = ((KBDLLHOOKSTRUCT *)lParam)->vkCode;
    switch (wParam)
    {
    case WM_KEYDOWN:
        if (process_keydown(keycode))
            return TRUE;
        break;
    case WM_KEYUP:
        if (process_keyup(keycode))
            return TRUE;
        break;
    default:
        break;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

class CapsHotkey
{
public:
    CapsHotkey()
    {
        hook_ = SetWindowsHookEx(WH_KEYBOARD_LL, process_hook_keyboard, NULL, 0);
        assert(hook_);
    }

    ~CapsHotkey()
    {
        UnhookWindowsHookEx(hook_);
    }

public:
    void register_hook(std::string_view name, const KeyHookFunc &func)
    {
        name2func_[std::string(name)] = func;
    }

private:
    HHOOK hook_{ 0 };
};
