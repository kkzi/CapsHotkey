#include "CapsHotkey.hpp"
#include "resource.h"
#include <Windows.h>
#include <iostream>

static constexpr const char *APP_ID{ "Caps Hotkey v1" };
static HWND window_{ 0 };
static HICON icon_{ 0 };

static auto create_notification_icon(HWND hwnd)
{
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = icon_;
    lstrcpy(nid.szTip, APP_ID);
    Shell_NotifyIcon(NIM_ADD, &nid);
}

static auto destory_notification_icon(HWND hwnd)
{
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

static auto show_context_menu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hPopupMenu = CreatePopupMenu();
    // AppendMenu(hPopupMenu, MF_STRING, 2, "Help");
    AppendMenu(hPopupMenu, MF_STRING, 1, "Exit");

    // Track the context menu
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
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

static auto process_cmd_message(HWND hwnd, WPARAM param)
{
    switch (LOWORD(param))
    {
    case 1:
        DestroyWindow(hwnd);
        break;
    case 2:
        // DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, HelpDialogProc);
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
        process_cmd_message(hwnd, wParam);
        break;
    case WM_DESTROY:
        destory_notification_icon(hwnd);
        PostQuitMessage(0);
        break;
    case WM_USER + 1:
        process_notification_icon_message(hwnd, lParam);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static auto create_main_window(HINSTANCE hinst) -> HWND
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = process_message;
    wc.hInstance = hinst;
    wc.lpszClassName = "CapsHotkeyWindowClass";
    wc.hIcon = icon_;
    RegisterClass(&wc);

    return CreateWindow(wc.lpszClassName, APP_ID, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, hinst, NULL);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    CapsHotkey hotkey;
    hotkey.register_hook("$quit_current_app", [] { DestroyWindow(window_); });
    hotkey.register_hook("$show_help_window", [] { ShowWindow(window_, SW_SHOW); });

    icon_ = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    window_ = create_main_window(hInst);
    ShowWindow(window_, SW_HIDE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(window_);
    // logfile_.close();
    return EXIT_SUCCESS;
}
