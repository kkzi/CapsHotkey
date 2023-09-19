#include <Windows.h>

#include "MainWindowWtl.hpp"
#include "res/resource.h"

CAppModule _Module;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    auto logo = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON2));
    wtl::Run<MainWindow>(hInst, SW_HIDE, logo);
    DestroyIcon(logo);
    return EXIT_SUCCESS;
}
