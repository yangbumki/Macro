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
    ActivateMacro am(10);
    Recorder rc;

   byte input;

    while (1) {
        cout << "======BGY_MACRO======" << endl;
        cout << "1. Recording" << endl;
        cout << "2. Macro Start" << endl;
        cout << "3. Exit" << endl<<endl;

        cout << "INPUT : "; cin >> input;

        switch (input) {
        case '1': {
            rc.ResetRecordData();

            cout << "Recording Start" << endl;
            cout << "If you want to end the macro" << endl;
            cout << "Press \"ESC\"" << endl;
            rc.Recording();

            while (1) {
                if (_kbhit()) {
                    if (_getch() == ESC) {
                        rc.End();
                        break;
                    }
                }
            }

            auto rcDatas = rc.GetRecordData();

            cout << "Saving Recording Data" << endl;

            for (auto& data : rcDatas) {
                if (data.vkCode == VK_ESCAPE) break;
                if (!am.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode, data.scanCode)) {
                    break;
                }
            }
            break;
        }
        case '2':
            am.MacroStart();
            cout << "Macro Start : after 3 seconds" << endl;
            Sleep(3000);
            
            am.MacroRun();

            cout << "If you want to end the macro" << endl;
            cout << "Press \"ESC\"" << endl;
            while (1) {
                if (_kbhit()) {
                    if (_getch() == ESC) {
                        am.MacroStop();
                        break;
                    }
                }
            }
            break;
        case '3':
            cout << "Closing..." << endl;
            goto EXIT;
            break;
        default:
            cout << "Invalid Value"<< endl;
        }
    }

EXIT:
    return 0;
}