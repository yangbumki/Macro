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
#define RECORDER                1

#define TICK_TIME               1000

using namespace std;

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