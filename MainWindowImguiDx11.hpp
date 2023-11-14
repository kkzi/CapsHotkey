#pragma once

#include "CapsHotkeyApp.hpp"
#include "res/resource.h"
#include <memory>

#if 0
    #include <simple/use_imgui_dx9.hpp>
#else
    #include <simple/use_imgui_dx11.hpp>
#endif

static auto APP_ID = L"Capslock Hotkey";
static auto APP_VERSION = L"v2.10";
static auto run_imgui_loop(HINSTANCE inst)
{
    static std::shared_ptr<CapsHotkeyApp> app = nullptr;
    static auto icon = LoadIcon(inst, MAKEINTRESOURCE(IDI_ICON2));
    static int last_selected_key = 0;

    ImGuiDx::OnMessage(WM_CREATE, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        app = std::make_shared<CapsHotkeyApp>(icon, hWnd);
        return 0;
    });
    ImGuiDx::OnMessage(WM_COMMAND, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        switch ((MenuAction)wParam)
        {
        case MenuAction::Help:
            app->show_main_window();
            break;
        case MenuAction::AutoRun:
            app->toggle_autorun_enabled();
            break;
        case MenuAction::Exit:
            app->quit_current_app();
            break;
        default:
            break;
        }
        return 0;
    });
    ImGuiDx::OnMessage(WM_USER + 1, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (lParam == WM_RBUTTONUP)
            app->show_context_menu();
        return 0;
    });
    ImGuiDx::OnMessage(WM_CLOSE, [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        app->show_main_window();
        return 0;
    });

    ImGuiDx::RunOptions opts;
    opts.Icon = icon;
    opts.Title = APP_ID;
    opts.Width = 640;
    opts.Height = 580;
    opts.CmdShow = SW_HIDE;

    ImGuiDx::Run(opts, [](auto &&win) -> bool {
        auto caption = std::format(L"{} {} Mappings\n", APP_ID, APP_VERSION);
        ImGui::Text(str::narrow(caption).data());
        auto flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable("MAPPINGS", 2, flags, { 0, 400 }))
        {
            ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 140);
            ImGui::TableSetupColumn("Desc.", ImGuiTableColumnFlags_WidthStretch, 0);
            ImGui::TableHeadersRow();

            for (auto &&[key, item] : key2hook_)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                auto label = std::format("[Capslock] + {}", KeyCodes::key2str(item.source)).c_str();
                auto selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                const bool item_is_selected = last_selected_key == key;
                if (ImGui::Selectable(label, item_is_selected, selectable_flags, ImVec2(0, 0)))
                {
                    last_selected_key = key;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(item.desc.c_str());
            }
            ImGui::EndTable();
        }
        ImGui::Text("\n");
        ImGui::Text("Open Source");
        ImGui::Bullet();
        if (ImGui::Button("kkzi/CapsHotkey"))
        {
            ShellExecuteA(NULL, "open", "https://github.com/kkzi/capshotkey", NULL, NULL, SW_SHOWNORMAL);
        }
        ImGui::Bullet();
        if (ImGui::Button("ocornut/imgui"))
        {
            ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, SW_SHOWNORMAL);
        }
        return true;
    });
}
