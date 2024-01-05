#pragma comment(linker, "/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

//#include "MainWindowW32Wrap.hpp"
// run_mainwindow_use_win32_wrap(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

//#include "MainWindowImguiDx9.hpp"
//#include "MainWindowImguiDx11.hpp"

#define TRAY_WINAPI 1

#include "CapsHotkeyApp.hpp"
#include <filesystem>
#include <simple/str.hpp>
#include <simple/tray.hpp>
#include <simple/use_win32.hpp>

std::wstring appid_{ L"Capslock Hotkey" };
std::wstring subkey_{ L"Software\\Microsoft\\Windows\\CurrentVersion\\Run" };

void load_hotkey_from_cfg(CapsHotkey &hotkey)
{
    static auto read_cfg = [&hotkey](auto &&text) {
        auto parts = str::split(text, "\r", false);
        for (auto &&it : parts)
        {
            hotkey.register_hook(it);
        }
    };

    // load default config
    if (auto res = FindResource(NULL, MAKEINTRESOURCE(IDR_CFG2), TEXT("CFG")); res > 0)
    {
        if (auto rr = LoadResource(NULL, res); rr > 0)
        {
            if (auto locked = (char *)LockResource(rr); locked != nullptr)
            {
                std::string text(locked, SizeofResource(NULL, res));
                read_cfg(text);
                FreeResource(locked);
            }
        }
    }

    // load user config
    {
        std::ifstream in("caps.cfg");
        std::string text({ std::istreambuf_iterator<char>(in), {} });
        read_cfg(text);
    }
}

std::wstring get_app_path()
{
    std::wstring path(MAX_PATH, 0);
    GetModuleFileName(NULL, path.data(), (DWORD)path.size());
    return path.c_str();
};

bool is_auto_run()
{
    HkeyUser hku(subkey_.c_str(), KEY_READ);
    auto left = str::trim_copy(hku.read(appid_));
    auto right = str::trim_copy(get_app_path());

    if (left.empty())
        return false;

    return left.find(right) != std::wstring::npos || right.find(left) != std::wstring::npos;
}

void toggle_autorun_enabled()
{
    HkeyUser hku(subkey_.c_str(), KEY_WRITE);
    auto need_set = !is_auto_run();
    hku.remove(std::wstring_view{ appid_ });
    if (need_set)
    {
        hku.write(appid_, get_app_path());
    }
}

void run_tray_loop(HICON icon)
{
    tray_icon tray;
    auto quit = [&tray] { tray.quit(); };
    tray.initialize(icon, {
                              tray_menu{ L"v2.11" },
                              tray_menu{ L"-" },
                              tray_menu{ L"Autorun",
                                  [&tray](auto &item) {
                                      toggle_autorun_enabled();
                                      item.checked = is_auto_run();
                                      tray.update_tray();
                                  },
                                  is_auto_run() },
                              tray_menu{ L"Quit", [&quit](auto &) { quit(); } },
                          });

    CapsHotkey app;
    {
        app.register_hook(VK_OEM_7, simulate_mouse_up, "Mouse up");
        app.register_hook(VK_OEM_1, simulate_mouse_down, "Mouse down");
        app.register_hook(char2key('q'), quit, "Quit CapsHotkey");
        load_hotkey_from_cfg(app);
    }

    exec_main_loop();
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // run_imgui_loop(hInstance);

    auto icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    run_tray_loop(icon);
    return EXIT_SUCCESS;
}
