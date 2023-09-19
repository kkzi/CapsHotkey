#pragma comment(linker, "/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

#include "CapsHotkeyApp.hpp"
#include "res/resource.h"
#include "thWin32App.h"

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    static int span = 10;
    static int height = 24;

    auto logo = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
    thWin32App app;
    thForm form(logo, TEXT("Capslock Hotkey v2.7"));
    form.Width = 600;
    form.Height = 400;
    form.X = (GetSystemMetrics(SM_CXSCREEN) - form.Width) / 2;
    form.Y = (GetSystemMetrics(SM_CYSCREEN) - form.Height) / 2;

    thLabel label(&form, span, 6);
    label.Width = form.Width - span * 2;
    label.Height = height;
    label.Text = TEXT("Capslock Hotkey Mappings");
    label.Font.SetSize(12);

    thListView table(&form, span, span + height);
    table.Width = label.Width;
    table.Height = form.Height - label.Height - span * 2;
    table.Anchors.Right = true;
    table.Anchors.Bottom = true;
    table.Columns.Add(TEXT("Key"), 160);
    table.Columns.Add(TEXT("Description"), table.Width - span * 3 - 160);

    table.SetView(thListView::eViewType_t::view_details);

    CapsHotkeyApp cha(logo, form.GetHandle());
    auto i = 0;
    for (auto &&[k, it] : cha.hooks())
    {
        table.Items.Add(std::format(L"[Capslock] + {}", str::wide(KeyCodes::key2str(k))).c_str());
        table.Items[i++].SubItems[1].SetText(str::wide(it.desc).c_str());
    }

    form.UserMessage[WM_USER + 1] = [&cha](auto &&obj, auto &&arg) -> LRESULT {
        if (arg.lParam == WM_RBUTTONUP)
        {
            cha.show_context_menu();
        }
        return 0;
    };
    form.UserMessage[WM_ERASEBKGND] = [&cha](auto &&, auto &&) -> LRESULT { return (INT_PTR)GetStockObject(WHITE_BRUSH); };
    form.UserMessage[WM_CTLCOLORSTATIC] = [&cha](auto &&, auto &&) -> LRESULT { return (INT_PTR)GetStockObject(WHITE_BRUSH); };
    form.UserMessage[WM_COMMAND] = [&cha](auto &&obj, auto &&arg) -> LRESULT {
        switch (MenuAction(LOWORD(arg.wParam)))
        {
        case MenuAction::Exit:
            cha.quit_current_app();
            break;
        case MenuAction::Help:
            cha.show_main_window();
            break;
        case MenuAction::AutoRun:
            cha.toggle_autorun_enabled();
            break;
        }
        return 0;
    };

    app.Run();

    DestroyIcon(logo);
    return EXIT_SUCCESS;
}
