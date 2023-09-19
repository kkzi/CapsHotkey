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

class MainWindow : public CDialogImpl<MainWindow>, public CMessageFilter
{
public:
    enum
    {
        IDD = IDD_DIALOG1
    };

    BEGIN_MSG_MAP(MainWindow)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnBackgroundColor)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnBackgroundColor)
    MESSAGE_HANDLER(WM_USER + 1, OnUserCmd)
    END_MSG_MAP()

    // BEGIN_UPDATE_UI_MAP(MainWindow)
    // END_UPDATE_UI_MAP()

public:
    MainWindow(HICON icon)
        : icon_logo_(icon)
    {
        hotkey_.register_hook(VK_OEM_7, simulate_mouse_up, "Mouse up");
        hotkey_.register_hook(VK_OEM_1, simulate_mouse_down, "Mouse down");
        hotkey_.register_hook(
            VK_OEM_2, [this] { show_main_window(); }, "Show help window");
        hotkey_.register_hook(
            char2key('q'), [this] { quit_current_app(); }, "Quit CapsHotkey");
        load_hotkey_from_cfg();
    }

    ~MainWindow()
    {
    }

protected:
    BOOL PreTranslateMessage(MSG *pMsg) override
    {
        return ::IsDialogMessage(m_hWnd, pMsg);
    }

    int OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        create_notification_icon();

        SetIcon(icon_logo_, FALSE);
        CenterWindow();

        SetDlgItemText(IDC_STATIC, (appid_ + L" " + appver_).c_str());
        auto listvc = (CListViewCtrl)GetDlgItem(IDC_LIST1);
        listvc.SetExtendedListViewStyle(LVS_EX_TRACKSELECT | LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT);
        listvc.InsertColumn(0, _T("Key"), LVCFMT_LEFT | TVIS_BOLD, 100, 0);
        listvc.InsertColumn(1, _T("Description"), LVCFMT_LEFT, 400, 1);

        auto row = 0;
        for (auto &&[k, it] : hotkey_.hooks())
        {
            listvc.AddItem(row, 0, std::format(L"[Capslock] + {}", str::wide(KeyCodes::key2str(k))).c_str());
            listvc.AddItem(row, 1, str::wide(it.desc).c_str());
            row++;
        }
        auto loop = wtl::Loop();
        loop->AddMessageFilter(this);
        // loop->AddIdleHandler(this);

        return 0;
    }

    int OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    int OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL & /*bHandled*/)
    {
        switch (MenuAction(LOWORD(wParam)))
        {
        case MenuAction::Exit:
            quit_current_app();
            break;
        case MenuAction::Help:
            show_main_window();
            break;
        case MenuAction::AutoRun:
            toggle_autorun_enabled();
            break;
        }

        return 0;
    }

    int OnBackgroundColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        return (INT_PTR)GetStockObject(WHITE_BRUSH);
    }

    int OnUserCmd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL & /*bHandled*/)
    {
        if (lParam == WM_RBUTTONUP)
        {
            show_context_menu();
        }
        return 0;
    }

private:
    void create_notification_icon()
    {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = icon_logo_;
        lstrcpy(nid.szTip, appid_.c_str());
        Shell_NotifyIcon(NIM_ADD, &nid);
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
        auto hwnd = m_hWnd;
        SetForegroundWindow(hwnd);
        TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x - 5, pt.y - 5, 0, hwnd, NULL);
        PostMessage(WM_NULL, 0, 0);
    }

    void quit_current_app()
    {
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = m_hWnd;
        nid.uID = 1;
        Shell_NotifyIcon(NIM_DELETE, &nid);

        DestroyWindow();
        PostQuitMessage(0);
    }

    void show_main_window()
    {
        if (IsWindowVisible())
        {
            ShowWindow(SW_HIDE);
        }
        else
        {
            ShowWindow(SW_SHOW);
            SetForegroundWindow(m_hWnd);
            CenterWindow();
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

private:
    enum class MenuAction : uint8_t
    {
        Exit,
        Help,
        AutoRun,
    };

private:
    HICON icon_logo_{ 0 };
    std::wstring appid_{ L"Capslock Hotkey" };
    std::wstring appver_{ L"v2.5" };
    std::wstring subkey_{ L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" };
    CapsHotkey hotkey_;
};
