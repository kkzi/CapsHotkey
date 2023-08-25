#include "AppAutoRun.hpp"
#include "CapsHotkey.hpp"
#include "res/resource.h"
#include <iostream>
#include <simple/use_imgui_dx12.hpp>

constexpr auto APP_ID{ TEXT("Caps Hotkey v2") };

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

static auto get_bitmap(HICON icon) -> HBITMAP
{
    ICONINFO iconinfo;
    GetIconInfo(icon, &iconinfo);
    return iconinfo.hbmColor;
}

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
    ShowWindow(hwnd_, SW_SHOW);
}

static auto set_autorun_enabled()
{
    if (set_app_autorun(APP_ID, app_path_, !is_autorun_))
    {
        is_autorun_ = !is_autorun_;
    }
}

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

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    icon_logo_ = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    app_path_ = []() -> std::wstring {
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        return path;
    }();
    is_autorun_ = is_app_autorun(APP_ID);

    CapsHotkey hotkey;
    hotkey.register_hook("quit_current_app", quit_current_app);
    hotkey.register_hook("show_help_window", show_main_window);

    ImGuiDx12::OnMessage(WM_CREATE, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        create_notification_icon(hWnd);
        return 0;
    });
    ImGuiDx12::OnMessage(WM_COMMAND, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        process_action_msg(hwnd_, wParam);
        return 0;
    });
    ImGuiDx12::OnMessage(WM_USER + 1, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        process_notification_icon_message(hwnd_, lParam);
        return 0;
    });
    ImGuiDx12::OnMessage(WM_CLOSE, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        ShowWindow(hwnd_, SW_HIDE);
        return 0;
    });

    ImGuiDx12::RunOptions opts;
    opts.Icon = icon_logo_;
    opts.Title = APP_ID;
    opts.CmdShow = SW_HIDE;
    ImGuiDx12::Run(opts, [](auto &&win) -> bool {
        hwnd_ = win;

        ImGui::Text("Capslock Hotkey Mappings");
        if (ImGui::BeginTable("MAPPINGS", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("CAPSLOCK", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Ctrl", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Alt", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Shift", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Desc.", ImGuiTableColumnFlags_WidthStretch, 0);
            ImGui::TableHeadersRow();

            for (auto &&[_, item] : key2hook_)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("O");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(item.src_modified & KEY_CTRL ? "O" : "");
                ImGui::TableSetColumnIndex(2);
                ImGui::Text(item.src_modified & KEY_ALT ? "O" : "");
                ImGui::TableSetColumnIndex(3);
                ImGui::Text(item.src_modified & KEY_SHIFT ? "O" : "");
                ImGui::TableSetColumnIndex(4);
                ImGui::Text(key2str(item.src_keycode).c_str());
                ImGui::TableSetColumnIndex(5);
                ImGui::Text((item.desc.empty() ? key2str(item.targets.at(0)) : item.desc).c_str());
            }
            ImGui::EndTable();
        }
        return true;
    });

    // logfile_.close();
    DestroyIcon(icon_logo_);
    return EXIT_SUCCESS;
}
