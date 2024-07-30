#include <iostream>
#include <conio.h>
#include <vector>

#include <windows.h>

#include "ActivateMacro.h"
#include "DeactivateMacro.h"
#include "Recorder.h"

#define ESC     27

#define NONE_ACTIVATE_MACRO     0
#define ACTIVATE_MACRO          0
#define RECORDER                0

#define TICK_TIME               1000

using namespace std;

int main() {
    ActivateMacro am;
    Recorder rc;

    rc.Recording();

    cout << "===Recording===" << endl;
    Sleep(5000);
    cout << "===End===" << endl;

    auto rcDatas = rc.GetRecordData();
    
    for (auto& data : rcDatas) {
        am.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode);
    }

    cout << "===Macro Start===" << endl;
    am.MacroRun();

    while (1) {
        
        if (_kbhit()) {
            if (_getch() == ESC) break;
        }
    }

    return 0;
}

#if NONE_ACTIVATE_MACRO
int main() {

    ActivateMacro am(1000);

    am.RegisterMacroKey('A');
    am.RegisterMacroKey('A', true);

    float currentTime = 0.0,
        lastTime = 0.0,
        tickTime = TICK_TIME;

    am.MacroStart();


    auto inputs = am.GetRegisterInputs();

    while (1) {
        Sleep(TICK_TIME);
        am.MacroRun();
        cout << "Macro running" << endl;

        if (_kbhit()) {
            if (_getch() == ESC) {
                break;
            }
        }
    }

    return 0;
}
#endif

#if RECORDER
int main() {
    Recorder rc;

    rc.Recording();

    while (true) {

        if (_kbhit()) {
            if (_getch() == ESC) break;
        }
    }
    return 0;
}
#endif