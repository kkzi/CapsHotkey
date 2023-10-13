#pragma comment(linker, "/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

//#include "MainWindowW32Wrap.hpp"
// run_mainwindow_use_win32_wrap(hInstance, hPrevInstance, lpCmdLine, nShowCmd);

//#include "MainWindowImguiDx9.hpp"
#include "MainWindowImguiDx11.hpp"

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    run_imgui_loop(hInstance);
    return EXIT_SUCCESS;
}
