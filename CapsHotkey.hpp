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
static constexpr duration PRESS_TIMEOUT{ 300ms };

struct KeyHookItem
{
    int source{ 0 };
    std::vector<std::vector<int>> target_keys{};
    KeyHookFunc target_func{ nullptr };
    std::string desc;
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

static auto is_key_pressed(int key) -> bool
{
    return GetKeyState(key) & 0x8000;
}

static void reset_hook_status();
static void key_up(int key)
{
    log("--keyup {}", key2str(key));

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
    log("--key click: {}", txt);
    auto n = SendInput((UINT)len, inputs, (UINT)sizeof(INPUT));
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

static std::vector<KeyHookItem> key_hooks_{
    { char2key('h'), { { VK_LEFT } } },
    { char2key('j'), { { VK_DOWN } } },
    { char2key('k'), { { VK_UP } } },
    { char2key('l'), { { VK_RIGHT } } },
    { char2key('n'), { { VK_DOWN } } },
    { char2key('p'), { { VK_UP } } },
    { char2key('a'), { { VK_HOME } } },
    { char2key('e'), { { VK_END } } },
    { char2key('b'), { { VK_CONTROL, VK_LEFT } } },
    { char2key('f'), { { VK_CONTROL, VK_RIGHT } } },

    { char2key('w'), { { VK_LCONTROL, VK_LSHIFT, VK_LEFT }, { VK_LCONTROL, VK_INSERT }, { VK_DELETE } }, nullptr, "Delete word backforward" },
    { char2key('u'), { { VK_SHIFT, VK_HOME }, { VK_CONTROL, VK_INSERT }, { VK_DELETE } }, nullptr, "Delete from cursor to line begin" },
    { char2key('c'), { { VK_SHIFT, VK_END }, { VK_CONTROL, VK_INSERT }, { VK_DELETE } }, nullptr, "" },
    { char2key('s'), { { VK_HOME }, { VK_SHIFT, VK_END }, { VK_CONTROL, VK_INSERT }, { VK_DELETE } }, nullptr, "Delete current line" },
    { char2key('o'), { { VK_HOME }, { VK_RETURN }, { VK_UP } }, nullptr, "Insert new blank line before current" },
    //{ char2key('o'), { { VK_END, VK_RETURN } }, nullptr, "Insert new blank line after current" },

    { char2key('y'), { { VK_CONTROL, VK_INSERT } } },
    { char2key('p'), { { VK_SHIFT, VK_INSERT } } },
};

static std::map<int, KeyHookItem> key2hook_ = ([] {
    std::map<int, KeyHookItem> key2hook;
    for (auto it : key_hooks_)
    {
        if (it.desc.empty())
        {
            std::stringstream ss;
            for (auto &&one : it.target_keys)
            {
                for (auto &&k : one)
                {
                    ss << key2str(k) << " ";
                }
                ss << ", ";
            }
            it.desc = ss.str();
        }
        key2hook[it.source] = it;
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
    if (item.target_func != nullptr)
    {
        item.target_func();
        return;
    }

    for (auto &&one : item.target_keys)
    {
        key_click(one);
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
    // if (key == VK_LSHIFT && !keydown && status_ == HookStatus::Hooking)
    //{
    //    log("ignore {} {}", key2str(key), state);
    //    return true;
    //}

    if (modified_pressed_.contains(key))
    {
        auto old = modified_pressed_[key] == keydown;
        modified_pressed_[key] = keydown;
        auto ignored = old && status_ == HookStatus::Hooking;
        if (ignored)
        {
            log("ignore {} {}", key2str(key), state);
        }
        return ignored;
    }

    if (keydown && status_ != HookStatus::Hooking && key2hook_.contains(key))
    {
        log("hook {} {}", key2str(key), state);
        status_ = HookStatus::Hooking;
        process_hotkey(key2hook_.at(key));
        status_ = HookStatus::Hooked;
        log("hook down");
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
    log("##key {} {}", key2str(keycode), wParam == WM_KEYUP ? "up" : "down");
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
    void register_hook(int keycode, const KeyHookFunc &func, std::string_view desc)
    {
        key2hook_[keycode] = KeyHookItem{ keycode, {}, func, std::string{ desc } };
    }

private:
    HHOOK hook_{ 0 };
};
