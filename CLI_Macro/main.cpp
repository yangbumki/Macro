#include <iostream>
#include <conio.h>
#include <vector>

#include <windows.h>

#define ESC     27

#define NONE_ACTIVATE_MACRO     1
#define ACTIVATE_MACRO          0

using namespace std;

#if NONE_ACTIVATE_MACRO
int main() {

    vector<INPUT> inputs;

    INPUT input;

    input.ki.wVk = VK_LSHIFT;
    input.type = INPUT_KEYBOARD;

    /*inputs.push_back(input);*/

    ZeroMemory(&input, sizeof(INPUT));

    input.ki.wVk = 'E';
    input.type = INPUT_KEYBOARD;

    inputs.push_back(input);
    
    byte idx = 0;

    while (1) {
        Sleep(100);

        for (auto& input : inputs) {
            SendInput(1, &input, sizeof(INPUT));
        }

        if (_kbhit()) {
            
            if (_getch() == ESC) {
                break;
            }
        }
    }

    return 0;
}
#elif ACTIVATE_MACRO
ULONG ProcIDFromWnd(HWND hwnd) // 윈도우 핸들로 프로세스 아이디 얻기  
{
    ULONG idProc;
    GetWindowThreadProcessId(hwnd, &idProc);
    return idProc;
}

HWND GetWinHandle(ULONG pid) // 프로세스 아이디로 윈도우 핸들 얻기  
{
    HWND tempHwnd = FindWindow(NULL, NULL); // 최상위 윈도우 핸들 찾기  

    while (tempHwnd != NULL)
    {
        if (GetParent(tempHwnd) == NULL) // 최상위 핸들인지 체크, 버튼 등도 핸들을 가질 수 있으므로 무시하기 위해  
            if (pid == ProcIDFromWnd(tempHwnd))
                return tempHwnd;
        tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT); // 다음 윈도우 핸들 찾기  
    }
    return NULL;
}

int main() {
    auto arkHwnd = FindWindow(L"UnrealWindow", L"ARK: Survival Evolved");
    if (arkHwnd == NULL) {
        return -1;
    }

    while (1) {
        Sleep(100);
        SendMessage(arkHwnd, WM_ACTIVATE, 1, NULL);
        SendMessage(arkHwnd, WM_KEYDOWN, VK_LSHIFT, VK_LSHIFT);
        SendMessage(arkHwnd, WM_KEYDOWN, 0x57, 0x57);
        if (_kbhit()) {
            if (_getch() == ESC) break;
        }
    }
    return 0;
}
#endif