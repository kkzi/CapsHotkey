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

struct KeyHookItem
{
    std::string desc;
    int src_keycode{ 0 };
    uint8_t tar_modified{ 0 };
    std::vector<int> targets;
    std::string func;
};

enum class HookStatus
{
    Normal = 0,
    Hooking = 1,
    Hooked = 2,
};

static HookStatus status_{ HookStatus::Normal };
static bool capslock_busy_{ false };
static bool capslock_pressed_{ false };
static std::optional<KeyHookItem> capslock_hook_{ std::nullopt };
static std::optional<time_point<system_clock>> capslock_pressed_time_{ std::nullopt };

static std::map<int, bool> modified_pressed_{
    { VK_LCONTROL, false },
    { VK_LMENU, false },
    { VK_LSHIFT, false },
    { VK_LWIN, false },
};

template <class T, class... Args>
static void log(const T *fmt, Args &&...args)
{
    auto line = std::vformat(fmt, std::make_format_args(args...));
    OutputDebugStringA(line.c_str());
    OutputDebugStringA("\n");
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

static void reset_hook_status();
static void key_up(int key)
{
    log("-- keyup {}", key2str(key));

    if (key == KEY_CAPSLOCK)
    {
        reset_hook_status();
    }
    if (modified_pressed_.contains(key))
    {
        modified_pressed_[key] = false;
    }

    INPUT inputs[1]{};
    inputs[0].type = 1;
    inputs[0].ki.wVk = (short)key;
    inputs[0].ki.dwFlags = 2;
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

static void key_down(int key)
{
    log("-- keydown {}", key2str(key));
    INPUT inputs[1]{};
    inputs[0].type = 1;
    inputs[0].ki.wVk = (short)key;
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

static void key_click(const std::vector<int> &arr)
{
    if (arr.empty())
        return;

    auto len = arr.size() * 2;
    auto inputs = new INPUT[len]{ 0 };
    std::string txt;
    for (auto i = 0; i < arr.size(); ++i)
    {
        auto key = arr[i];
        txt += key2str(key);
        auto upi = len - 1 - i;

        inputs[i].type = 1;
        inputs[i].ki.wVk = key;

        inputs[upi].type = 1;
        inputs[upi].ki.wVk = key;
        inputs[upi].ki.dwFlags = KEYEVENTF_KEYUP;
    }
    log("!!{}, hooking={}", txt, capslock_hook_.has_value());
    auto n = SendInput(len, inputs, sizeof(INPUT));
    delete[] inputs;
}

static void reset_modifies()
{
    for (auto &&[k, v] : modified_pressed_)
    {
        if (v)
        {
            key_up(k);
        }
    }
}

static void reset_hook_status()
{
    reset_modifies();
    capslock_pressed_time_ = std::nullopt;
    capslock_pressed_ = false;
    status_ = HookStatus::Normal;
}

static void key_click(int key)
{
    key_click(std::vector<int>{ key });
}

static std::map<std::string, KeyHookFunc> name2func_{
    {
        "delete_left_word",
        [] {
            key_click({ VK_LCONTROL, VK_LSHIFT, VK_LEFT });
            key_click({ VK_LCONTROL, VK_INSERT });
            key_click({ VK_DELETE });
        },
    },
    {
        "delete_to_line_begin",
        [] {
            key_click({ VK_SHIFT, VK_HOME });
            key_click({ VK_CONTROL, VK_INSERT });
            key_click({ VK_DELETE });
        },
    },
    {
        "delete_to_line_end",
        [] {
            key_click({ VK_SHIFT, VK_END });
            key_click({ VK_CONTROL, VK_INSERT });
            key_click({ VK_DELETE });
        },
    },
    {
        "delete_current_line",
        [] {
            key_click(VK_HOME);
            key_click({ VK_SHIFT, VK_END });
            key_click({ VK_CONTROL, VK_INSERT });
            key_click({ VK_DELETE });
        },
    },
    {
        "new_line_up",
        [] {
            key_click(VK_HOME);
            key_click(VK_RETURN);
            key_click(VK_UP);
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

static auto hook_capslock(WPARAM wParam) -> bool
{
    if (capslock_busy_)
    {
        return false;
    }
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
    {
        capslock_pressed_ = true;
        if (!capslock_pressed_time_)
        {
            capslock_pressed_time_ = system_clock::now();
            log("capslock pressed");
        }
    }
    else
    {
        auto duration = duration_cast<milliseconds>(system_clock::now() - *capslock_pressed_time_);
        if (duration <= PRESS_TIMEOUT && status_ == HookStatus::Normal)
        {
            capslock_busy_ = true;
            key_click(KEY_CAPSLOCK);
            capslock_busy_ = false;
        }
        reset_hook_status();
    }
    return true;
}

static auto process_hotkey(const KeyHookItem &item)
{
    if (!item.func.empty() && name2func_.contains(item.func))
    {
        name2func_.at(item.func)();
        return;
    }

    if (item.targets.empty())
    {
        return;
    }

    for (auto &&k : item.targets)
    {
        // var arr = new List<VirtualKey>();
        // if ((it.Modified & ModifiedKey.Ctrl) == ModifiedKey.Ctrl)
        //    arr.Add(VirtualKey.Control);
        // if ((it.Modified & ModifiedKey.Alt) == ModifiedKey.Alt)
        //    arr.Add(VirtualKey.Menu);
        // if ((it.Modified & ModifiedKey.Shift) == ModifiedKey.Shift)
        //    arr.Add(VirtualKey.LeftShift);
        // if ((it.Modified & ModifiedKey.Win) == ModifiedKey.Win)
        //    arr.Add(VirtualKey.LeftWindows);
        // arr.AddRange(it.Keys);
        key_click(item.targets);
    }
}

static auto hook_other_key(int key, WPARAM wParam) -> bool
{
    if (!capslock_pressed_)
    {
        return false;
    }

    auto keydown = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    auto state = keydown ? "down" : "up";
    if (key == VK_LSHIFT && !keydown && status_ == HookStatus::Hooking)
    {
        return true;
    }

    if (modified_pressed_.contains(key))
    {
        auto old = modified_pressed_[key] == keydown;
        modified_pressed_[key] = keydown;
        return old && status_ == HookStatus::Hooking;
    }

    if (keydown && status_ != HookStatus::Hooking && key2hook_.contains(key))
    {
        status_ = HookStatus::Hooking;
        process_hotkey(key2hook_.at(key));
        status_ = HookStatus::Hooked;
        return true;
    }
    return false;
}

static auto process_hook_keyboard(int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT
{
    if (nCode != HC_ACTION)
    {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    auto keycode = ((KBDLLHOOKSTRUCT *)lParam)->vkCode;
    if (keycode == KEY_CAPSLOCK)
    {
        if (hook_capslock(wParam))
        {
            return TRUE;
        }
    }
    else
    {
        if (hook_other_key(keycode, wParam))
        {
            return TRUE;
        }
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
