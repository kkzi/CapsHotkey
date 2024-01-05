#ifndef TRAY_H
#define TRAY_H

#include <functional>
#include <list>
#include <string>

#include <Windows.h>

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)
#define WC_TRAY_CLASS_NAME "TRAY"
#define ID_TRAY_FIRST 1000

static HWND hwnd_;
static HMENU hmenu_;

struct tray_menu
{
    std::wstring text;
    std::function<void(tray_menu &)> process;
    bool checked{ false };
    bool disabled{ false };
    std::list<tray_menu> menus;
};

class tray_icon
{
public:
    tray_icon(HICON icon)
        : icon_(icon)
    {
    }

public:
    void init_menus(std::initializer_list<tray_menu> menus)
    {
        menus_ = menus;
        init_tray();
        update_tray();
    }

    void update_tray()
    {
        HMENU prevmenu = hmenu_;
        UINT id = ID_TRAY_FIRST;
        hmenu_ = create_menu(menus_, &id);
        SendMessage(hwnd_, WM_INITMENUPOPUP, (WPARAM)hmenu_, 0);
        // HICON icon;
        // ExtractIconEx(icon_.c_str(), 0, NULL, &icon, 1);
        // if (nid_.hIcon)
        //{
        //    DestroyIcon(nid_.hIcon);
        //}
        nid_.hIcon = icon_;
        Shell_NotifyIcon(NIM_MODIFY, &nid_);
        if (prevmenu != NULL)
        {
            DestroyMenu(prevmenu);
        }
    }

    void quit()
    {
        Shell_NotifyIcon(NIM_DELETE, &nid_);
        if (nid_.hIcon != 0)
        {
            DestroyIcon(nid_.hIcon);
        }
        if (hmenu_ != 0)
        {
            DestroyMenu(hmenu_);
        }
        PostQuitMessage(0);
        UnregisterClass(TEXT(WC_TRAY_CLASS_NAME), GetModuleHandle(NULL));
    }

private:
    bool init_tray()
    {
        WNDCLASSEX wc{};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = tray_window_proc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = TEXT(WC_TRAY_CLASS_NAME);
        if (!RegisterClassEx(&wc))
        {
            return false;
        }

        hwnd_ = CreateWindowEx(0, TEXT(WC_TRAY_CLASS_NAME), NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        if (hwnd_ == NULL)
        {
            return false;
        }
        UpdateWindow(hwnd_);

        memset(&nid_, 0, sizeof(nid_));
        nid_.cbSize = sizeof(NOTIFYICONDATA);
        nid_.hWnd = hwnd_;
        nid_.uID = 0;
        nid_.uFlags = NIF_ICON | NIF_MESSAGE;
        nid_.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
        Shell_NotifyIcon(NIM_ADD, &nid_);

        return true;
    }

    static HMENU create_menu(const std::list<tray_menu> &menus, UINT *id)
    {
        if (menus.empty())
            return 0;

        HMENU hmenu = CreatePopupMenu();
        for (auto &&m : menus)
        {
            if (m.text.starts_with('-'))
            {
                InsertMenu(hmenu, (*id)++, MF_SEPARATOR, TRUE, L"");
            }
            else
            {
                MENUITEMINFO item{};
                item.cbSize = sizeof(MENUITEMINFO);
                item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
                item.fType = 0;
                item.fState = 0;
                item.fMask = item.fMask | MIIM_SUBMENU;
                item.hSubMenu = create_menu(m.menus, id);
                if (m.disabled)
                {
                    item.fState |= MFS_DISABLED;
                }
                if (m.checked)
                {
                    item.fState |= MFS_CHECKED;
                }
                item.wID = *id;
                item.dwTypeData = (wchar_t *)m.text.c_str();
                item.dwItemData = (ULONG_PTR)&m;
                InsertMenuItem(hmenu, (*id)++, TRUE, &item);
            }
        }
        return hmenu;
    }

    static LRESULT CALLBACK tray_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_TRAY_CALLBACK_MESSAGE:
            if (lparam == WM_LBUTTONUP || lparam == WM_RBUTTONUP)
            {
                POINT p;
                GetCursorPos(&p);
                SetForegroundWindow(hwnd);
                WORD cmd = TrackPopupMenu(hmenu_, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, p.x, p.y, 0, hwnd, NULL);
                SendMessage(hwnd, WM_COMMAND, cmd, 0);
                return 0;
            }
            break;
        case WM_COMMAND:
            if (wparam >= ID_TRAY_FIRST)
            {
                MENUITEMINFO item = {
                    .cbSize = sizeof(MENUITEMINFO),
                    .fMask = MIIM_ID | MIIM_DATA,
                };
                if (GetMenuItemInfo(hmenu_, wparam, FALSE, &item))
                {
                    struct tray_menu *menu = (struct tray_menu *)item.dwItemData;
                    if (menu != NULL && menu->process != NULL)
                    {
                        menu->process(*menu);
                    }
                }
                return 0;
            }
            break;
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

private:
    HICON icon_{ 0 };
    std::list<tray_menu> menus_;

    NOTIFYICONDATA nid_{};
};

static int tray_loop(int blocking)
{
    MSG msg;
    if (blocking)
    {
        GetMessage(&msg, NULL, 0, 0);
    }
    else
    {
        PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    }
    if (msg.message == WM_QUIT)
    {
        return -1;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return 0;
}

#endif /* TRAY_H */
