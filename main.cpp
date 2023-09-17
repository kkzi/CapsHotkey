#include "AppAutoRun.hpp"
#include "CapsHotkey.hpp"
#include "res/resource.h"
#include <filesystem>
#include <iostream>
#include <locale>
#include <ranges>
#include <simple/str.hpp>
#include <simple/use_wxx.hpp>

constexpr auto APP_ID{ TEXT("Capslock Hotkey\0") };
constexpr auto APP_VERSION{ TEXT("v2.3\0") };

static auto create_notification_icon(HWND hwnd)
{
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = icon_logo_;
    lstrcpy(nid.szTip, APP_ID);
    Shell_NotifyIcon(NIM_ADD, &nid);
}

enum class MenuAction : uint8_t
{
    Exit,
    Help,
    AutoRun,
};

static std::wstring app_path_;
static bool is_autorun_{ false };
static HWND hwnd_{ 0 };
static HICON icon_logo_{ 0 };

static auto quit_current_app()
{
    static auto destory_notification_icon = [](HWND hwnd) {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = 1;
        Shell_NotifyIcon(NIM_DELETE, &nid);
    };

    destory_notification_icon(hwnd_);
    PostQuitMessage(0);
}

static auto show_main_window()
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

static auto simulate_mouse_down()
{
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -WHEEL_DELTA * 3, 0);
}

static auto simulate_mouse_up()
{
    mouse_event(MOUSEEVENTF_WHEEL, 0, 0, WHEEL_DELTA * 3, 0);
}

static auto set_autorun_enabled()
{
    if (set_app_autorun(APP_ID, app_path_.data(), !is_autorun_))
    {
        is_autorun_ = !is_autorun_;
    }
}

static auto show_context_menu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    auto menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::Help, TEXT("Help"));
    AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::AutoRun, TEXT("Auto Run"));
    AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
    AppendMenu(menu, MF_STRING, (uint16_t)MenuAction::Exit, TEXT("Exit"));

    CheckMenuItem(menu, (uint16_t)MenuAction::AutoRun, is_autorun_ ? MF_CHECKED : MF_UNCHECKED);

    // Track the context menu
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x - 5, pt.y - 5, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
}

static auto process_notification_icon_message(HWND hwnd, LPARAM param)
{
    switch (param)
    {
    case WM_RBUTTONDOWN:
        show_context_menu(hwnd);
        break;
    }
}

static auto process_action_msg(HWND hwnd, WPARAM param)
{
    switch (MenuAction(LOWORD(param)))
    {
    case MenuAction::Exit:
        quit_current_app();
        break;
    case MenuAction::Help:
        show_main_window();
        break;
    case MenuAction::AutoRun:
        set_autorun_enabled();
        break;
    }
}

LRESULT CALLBACK process_message(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        create_notification_icon(hwnd);
        break;
    case WM_COMMAND:
        process_action_msg(hwnd, wParam);
        break;
    case WM_DESTROY:
        quit_current_app();
        break;
    case WM_USER + 1:
        process_notification_icon_message(hwnd, lParam);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

auto load_hotkey_from_cfg()
{
    static auto read_line = [](auto &&line) {
        if (line.empty())
            return;
        auto hash_pos = line.find('#');
        if (hash_pos == 0)
            return;
        std::string left, desc;
        if (hash_pos == std::string::npos)
        {
            left = line;
        }
        else
        {
            left = str::trim_copy(line.substr(0, hash_pos));
            desc = str::trim_copy(line.substr(hash_pos + 1));
        }
        auto parts = str::split(left, "=", false);
        if (parts.size() < 2)
            return;

        bool invalid = false;
        KeyHookItem hook;
        hook.source = KeyCodes::str2key(str::trim_copy(parts.at(0)));
        hook.desc = desc.empty() ? parts.at(1) : desc;
        invalid = hook.source < 0;

        str::trim(hook.desc);
        parts = str::split(parts.at(1), ",", false);
        for (auto &&it : parts)
        {
            std::vector<int> combine;
            auto arr = str::split(it, " ", false);
            std::transform(arr.begin(), arr.end(), std::back_inserter(combine), [&invalid](auto &&key) {
                auto code = KeyCodes::str2key(str::trim_copy(key));
                if (code < 0)
                    invalid = true;
                return code;
            });
            hook.target_keys.push_back(combine);
        }

        if (invalid)
            hook.desc = "!!INVALID: " + hook.desc;
        key2hook_[hook.source] = hook;
    };
    static auto read_cfg = [](auto &&text) {
        auto parts = str::split(text, "\r", false);
        for (auto &&it : parts)
        {
            read_line(it);
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

class CapsHotKeyHelpView : public CWnd
{
protected:
    BOOL Minimize()
    {
        return TRUE;
    }

    BOOL OnAbout()
    {
        return TRUE;
    }

    int OnCreate(CREATESTRUCT &create)
    {
        create_notification_icon(create.hwndParent);
        return 0;
    }

    BOOL OnCommand(WPARAM wparam, LPARAM)
    {
        return FALSE;
    }

    void OnDestroy()
    {
        // End the application when the window is destroyed
        ::PostQuitMessage(0);
    }

    LRESULT OnDpiChanged(UINT, WPARAM, LPARAM lparam)
    {
        LPRECT prc = reinterpret_cast<LPRECT>(lparam);
        SetWindowPos(0, *prc, SWP_SHOWWINDOW);

        return 0;
    }

    void OnDraw(CDC &dc)
    {
        if (GetWinVersion() >= 2601)
        {
            NONCLIENTMETRICS info = GetNonClientMetrics();
            LOGFONT lf = DpiScaleLogfont(info.lfMessageFont, 10);
            dc.CreateFontIndirect(lf);
        }

        // Centre some text in our view window
        CRect rc = GetClientRect();
        CString text = "hello world";
        dc.DrawText(text, text.GetLength(), rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    BOOL OnFileExit()
    {
        // End the application
        Close();
        return TRUE;
    }

    void OnInitialUpdate()
    {
        // OnInitialUpdate is called after the window is created.
        // Tasks which are to be done after the window is created go here.

        TRACE("OnInitialUpdate\n");
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM)
    {
        // Force the window to be repainted during resizing
        Invalidate();
        return 0;
    }

    LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Maximize and Minimuze requests end up here

        if (wparam == SC_MINIMIZE)  // User pressed minimize button
        {
            Minimize();
            return 0;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    LRESULT OnTrayIcon(UINT, WPARAM wparam, LPARAM lparam)
    {
        // For a NOTIFYICONDATA with uVersion= 0, wparam and lparam have the following values:
        // The wparam parameter contains the identifier of the taskbar icon in which the event occurred.
        // The lparam parameter holds the mouse or keyboard message associated with the event.

        if (wparam != IDI_ICON2)
            return 0;

        if (lparam == WM_RBUTTONDOWN)
        {
            show_context_menu(GetHwnd());
        };

        return 0;
    }

    void PreCreate(CREATESTRUCT &cs)
    {
        // This function will be called automatically by Create. It provides an
        // opportunity to set various window parameters prior to window creation.
        // You are not required to set these parameters, any parameters which
        // aren't specified are set to reasonable defaults.

        // Set some optional parameters for the window
        cs.dwExStyle = WS_EX_CLIENTEDGE;  // Extended style
        cs.lpszClass = APP_ID;            // Window Class
        cs.x = DpiScaleInt(50);           // top x
        cs.y = DpiScaleInt(50);           // top y
        cs.cx = DpiScaleInt(400);         // width
        cs.cy = DpiScaleInt(300);         // height
        // cs.hMenu = m_menu;
    }

    LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        try
        {
            switch (msg)
            {
            case WM_DPICHANGED:
                return OnDpiChanged(msg, wparam, lparam);
            case WM_HELP:
                return OnAbout();
            case WM_SIZE:
                return OnSize(msg, wparam, lparam);
            case WM_SYSCOMMAND:
                return OnSysCommand(msg, wparam, lparam);
            case WM_USER:
                return OnTrayIcon(msg, wparam, lparam);
            }

            // Pass unhandled messages on for default processing.
            return WndProcDefault(msg, wparam, lparam);
        }

        // Catch all CException types.
        catch (const CException &e)
        {
            // Display the exception and continue.
            ::MessageBox(0, e.GetText(), AtoT(e.what()), MB_ICONERROR);
            return 0;
        }
    }
};

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    icon_logo_ = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    app_path_ = []() -> std::wstring {
        std::wstring path(MAX_PATH, 0);
        GetModuleFileName(NULL, path.data(), (DWORD)path.size());
        return path;
    }();
    is_autorun_ = is_app_autorun(APP_ID);

    CapsHotkey hotkey;
    hotkey.register_hook(VK_OEM_7, simulate_mouse_up, "Mouse up");
    hotkey.register_hook(VK_OEM_1, simulate_mouse_down, "Mouse down");
    hotkey.register_hook(VK_OEM_2, show_main_window, "Show help window");
    hotkey.register_hook(char2key('q'), quit_current_app, "Quit CapsHotkey");

    load_hotkey_from_cfg();

    wxx::App app(std::make_shared<CapsHotKeyHelpView>());
    app.Run();

    // MSG msg;
    // while (GetMessage(&msg, NULL, 0, 0) > 0)
    //{
    //    TranslateMessage(&msg);  //转换
    //    DispatchMessage(&msg);   //分发
    //}

    // logfile_.close();
    DestroyIcon(icon_logo_);
    return EXIT_SUCCESS;
}
