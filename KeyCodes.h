#pragma once

#include <WinUser.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <string>

namespace KeyCodes
{

    static auto str2key(const std::string &str) -> int
    {
        static std::map<std::string, int> mapping{
            { "LEFTBUTTON", VK_LBUTTON },
            { "RIGHTBUTTON", VK_RBUTTON },
            { "CANCEL", VK_CANCEL },
            { "MIDDLEBUTTON", VK_MBUTTON },
            { "EXTRABUTTON1", VK_XBUTTON1 },
            { "EXTRABUTTON2", VK_XBUTTON2 },
            { "BACK", VK_BACK },
            { "BACKSPACE", VK_BACK },
            { "TAB", VK_TAB },
            { "CLEAR", VK_CLEAR },
            { "RETURN", VK_RETURN },
            { "SHIFT", VK_SHIFT },
            { "CONTROL", VK_CONTROL },
            { "CTRL", VK_CONTROL },
            { "ALT", VK_MENU },
            { "MENU", VK_MENU },
            { "PAUSE", VK_PAUSE },
            { "CAPSLOCK", VK_CAPITAL },
            { "KANA", VK_KANA },
            { "HANGEUL", VK_KANA },
            { "HANGUL", VK_KANA },
            { "JUNJA", VK_JUNJA },
            { "FINAL", VK_FINAL },
            { "HANJA", VK_HANJA },
            { "KANJI", VK_HANJA },
            { "ESCAPE", VK_ESCAPE },
            { "CONVERT", VK_CONVERT },
            { "NONCONVERT", VK_NONCONVERT },
            { "ACCEPT", VK_ACCEPT },
            { "MODECHANGE", VK_MODECHANGE },
            { "SPACE", VK_SPACE },
            { "PRIOR", VK_PRIOR },
            { "NEXT", VK_NEXT },
            { "END", VK_END },
            { "HOME", VK_HOME },
            { "LEFT", VK_LEFT },
            { "UP", VK_UP },
            { "RIGHT", VK_RIGHT },
            { "DOWN", VK_DOWN },
            { "SELECT", VK_SELECT },
            { "PRINT", VK_PRINT },
            { "EXECUTE", VK_EXECUTE },
            { "SNAPSHOT", VK_SNAPSHOT },
            { "INSERT", VK_INSERT },
            { "DELETE", VK_DELETE },
            { "HELP", VK_HELP },
            { "N0", 0x30 },
            { "N1", 0x31 },
            { "N2", 0x32 },
            { "N3", 0x33 },
            { "N4", 0x34 },
            { "N5", 0x35 },
            { "N6", 0x36 },
            { "N7", 0x37 },
            { "N8", 0x38 },
            { "N9", 0x39 },
            { "A", 0x41 },
            { "B", 0x42 },
            { "C", 0x43 },
            { "D", 0x44 },
            { "E", 0x45 },
            { "F", 0x46 },
            { "G", 0x47 },
            { "H", 0x48 },
            { "I", 0x49 },
            { "J", 0x4A },
            { "K", 0x4B },
            { "L", 0x4C },
            { "M", 0x4D },
            { "N", 0x4E },
            { "O", 0x4F },
            { "P", 0x50 },
            { "Q", 0x51 },
            { "R", 0x52 },
            { "S", 0x53 },
            { "T", 0x54 },
            { "U", 0x55 },
            { "V", 0x56 },
            { "W", 0x57 },
            { "X", 0x58 },
            { "Y", 0x59 },
            { "Z", 0x5A },
            { "LEFTWINDOWS", VK_LWIN },
            { "RIGHTWINDOWS", VK_RWIN },
            { "APPLICATION", VK_APPS },
            { "SLEEP", VK_SLEEP },
            { "NUMPAD0", VK_NUMPAD0 },
            { "NUMPAD1", VK_NUMPAD1 },
            { "NUMPAD2", VK_NUMPAD2 },
            { "NUMPAD3", VK_NUMPAD3 },
            { "NUMPAD4", VK_NUMPAD4 },
            { "NUMPAD5", VK_NUMPAD5 },
            { "NUMPAD6", VK_NUMPAD6 },
            { "NUMPAD7", VK_NUMPAD7 },
            { "NUMPAD8", VK_NUMPAD8 },
            { "NUMPAD9", VK_NUMPAD9 },
            { "MULTIPLY", VK_MULTIPLY },
            { "ADD", VK_ADD },
            { "SEPARATOR", VK_SEPARATOR },
            { "SUBTRACT", VK_SUBTRACT },
            { "DECIMAL", VK_DECIMAL },
            { "DIVIDE", VK_DIVIDE },
            { "F1", VK_F1 },
            { "F2", VK_F2 },
            { "F3", VK_F3 },
            { "F4", VK_F4 },
            { "F5", VK_F5 },
            { "F6", VK_F6 },
            { "F7", VK_F7 },
            { "F8", VK_F8 },
            { "F9", VK_F9 },
            { "F10", VK_F10 },
            { "F11", VK_F11 },
            { "F12", VK_F12 },
            { "F13", VK_F13 },
            { "F14", VK_F14 },
            { "F15", VK_F15 },
            { "F16", VK_F16 },
            { "F17", VK_F17 },
            { "F18", VK_F18 },
            { "F19", VK_F19 },
            { "F20", VK_F20 },
            { "F21", VK_F21 },
            { "F22", VK_F22 },
            { "F23", VK_F23 },
            { "F24", VK_F24 },
            { "NUMLOCK", VK_NUMLOCK },
            { "SCROLLLOCK", VK_SCROLL },
            { "NEC_EQUAL", VK_OEM_NEC_EQUAL },
            { "FUJITSU_JISHO", VK_OEM_FJ_JISHO },
            { "FUJITSU_MASSHOU", VK_OEM_FJ_MASSHOU },
            { "FUJITSU_TOUROKU", VK_OEM_FJ_TOUROKU },
            { "FUJITSU_LOYA", VK_OEM_FJ_LOYA },
            { "FUJITSU_ROYA", VK_OEM_FJ_ROYA },
            { "LEFTSHIFT", VK_LSHIFT },
            { "RIGHTSHIFT", VK_RSHIFT },
            { "LEFTCONTROL", VK_LCONTROL },
            { "RIGHTCONTROL", VK_RCONTROL },
            { "LEFTMENU", VK_LMENU },
            { "RIGHTMENU", VK_RMENU },
            { "BROWSERBACK", VK_BROWSER_BACK },
            { "BROWSERFORWARD", VK_BROWSER_FORWARD },
            { "BROWSERREFRESH", VK_BROWSER_REFRESH },
            { "BROWSERSTOP", VK_BROWSER_STOP },
            { "BROWSERSEARCH", VK_BROWSER_SEARCH },
            { "BROWSERFAVORITES", VK_BROWSER_FAVORITES },
            { "BROWSERHOME", VK_BROWSER_HOME },
            { "VOLUMEMUTE", VK_VOLUME_MUTE },
            { "VOLUMEDOWN", VK_VOLUME_DOWN },
            { "VOLUMEUP", VK_VOLUME_UP },
            { "MEDIANEXTTRACK", VK_MEDIA_NEXT_TRACK },
            { "MEDIAPREVTRACK", VK_MEDIA_PREV_TRACK },
            { "MEDIASTOP", VK_MEDIA_STOP },
            { "MEDIAPLAYPAUSE", VK_MEDIA_PLAY_PAUSE },
            { "LAUNCHMAIL", VK_LAUNCH_MAIL },
            { "LAUNCHMEDIASELECT", VK_LAUNCH_MEDIA_SELECT },
            { "LAUNCHAPPLICATION1", VK_LAUNCH_APP1 },
            { "LAUNCHAPPLICATION2", VK_LAUNCH_APP2 },
            { "OEM1", VK_OEM_1 },
            { "OEMPLUS", VK_OEM_PLUS },
            { "OEMCOMMA", VK_OEM_COMMA },
            { "OEMMINUS", VK_OEM_MINUS },
            { "OEMPERIOD", VK_OEM_PERIOD },
            { "OEM2", VK_OEM_2 },
            { "OEM3", VK_OEM_3 },
            { "OEM4", VK_OEM_4 },
            { "OEM5", VK_OEM_5 },
            { "OEM6", VK_OEM_6 },
            { "OEM7", VK_OEM_7 },
            { "OEM8", VK_OEM_8 },
            { "OEMAX", VK_OEM_AX },
            { "OEM102", VK_OEM_102 },
            { "ICOHELP", VK_ICO_HELP },
            { "ICO00", VK_ICO_00 },
            { "PROCESSKEY", VK_PROCESSKEY },
            { "ICOCLEAR", VK_ICO_CLEAR },
            { "PACKET", VK_PACKET },
            { "OEMRESET", VK_OEM_RESET },
            { "OEMJUMP", VK_OEM_JUMP },
            { "OEMPA1", VK_OEM_PA1 },
            { "OEMPA2", VK_OEM_PA2 },
            { "OEMPA3", VK_OEM_PA3 },
            { "OEMWSCTRL", VK_OEM_WSCTRL },
            { "OEMCUSEL", VK_OEM_CUSEL },
            { "OEMATTN", VK_OEM_ATTN },
            { "OEMFINISH", VK_OEM_FINISH },
            { "OEMCOPY", VK_OEM_COPY },
            { "OEMAUTO", VK_OEM_AUTO },
            { "OEMENLW", VK_OEM_ENLW },
            { "OEMBACKTAB", VK_OEM_BACKTAB },
            { "ATTN", VK_ATTN },
            { "CRSEL", VK_CRSEL },
            { "EXSEL", VK_EXSEL },
            { "EREOF", VK_EREOF },
            { "PLAY", VK_PLAY },
            { "ZOOM", VK_ZOOM },
            { "NONAME", VK_NONAME },
            { "PA1", VK_PA1 },
            { "OEMCLEAR", VK_OEM_CLEAR },
        };

        std::string upper;
        std::transform(str.begin(), str.end(), std::back_inserter(upper), std::toupper);
        return mapping.contains(upper) ? mapping.at(upper) : -1;
    }

    static auto key2str(int vk) -> std::string
    {
        UINT scancode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
        CHAR text[128];
        int result = 0;
        switch (vk)
        {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_RCONTROL:
        case VK_RMENU:
        case VK_LWIN:
        case VK_RWIN:
        case VK_APPS:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_END:
        case VK_HOME:
        case VK_INSERT:
        case VK_DELETE:
        case VK_DIVIDE:
        case VK_NUMLOCK:
            scancode |= KF_EXTENDED;
        default:
            result = GetKeyNameTextA(scancode << 16, text, 128);
        }
        if (result == 0)
            _itoa_s((int)vk, text, 10);
        return text;
    }
}  // namespace KeyCodes
