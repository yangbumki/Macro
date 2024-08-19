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

bool PrintFileLists(const string path);

int main() {
	ActivateMacro am(10);
	Recorder rc;

	unsigned int ctlVar = 0;

	byte input;

	while (1) {
		cout << "======BGY_MACRO======" << endl;
		cout << "1. Recording" << endl;
		cout << "2. Macro Start" << endl;
		cout << "3. Macro Save" << endl;
		cout << "4. Macro Load" << endl;
		cout << "9. Exit" << endl << endl;

		cout << "INPUT : "; cin >> input; cout << endl;

		switch (input) {
		case '1': {
			rc.ResetRecordData();
			am.Reset();

			cout << "Recording Start" << endl;
			cout << "If you want to end the macro" << endl;
			cout << "Press \"ESC\"" << endl;
			rc.Recording();

			while (1) {
				if (_kbhit()) {
					if (_getch() == ESC) {
						rc.Stop();
						break;
					}
				}
			}

			auto rcDatas = rc.GetRecordData();

			cout << "Mappling Recording Data" << endl;

			for (auto& data : rcDatas) {
				//2024-08-07 일단 첫 기록시 Enter 기록되는 거 로딩 막음
				if (data.recordingTime == 0 && data.vkCode == 13) continue;

				if (data.vkCode == VK_ESCAPE) break;
				if (!am.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode, data.scanCode)) {
					break;
				}
			}
			am.MacroUpdate();
			am.MacroStop();
			break;
		}
		case '2': {
			string input;

			cout << "Do you wnat to random macro? (Yes/No)" << endl;
			cout << "INPUT : "; cin >> input; cout << endl;
			cout << "Macro Start : after 3 seconds" << endl;
			Sleep(3000);
			if (input.compare("Yes") == 0) {
				am.MacroRandom();
			}
			else {
				am.MacroStart();
			}

			

			//2024-08-19 macro play 추가
			/*cout << "If you want to end the macro" << endl;
			cout << "Press \"ESC\"" << endl;
			while (1) {
				if (_kbhit()) {
					if (_getch() == ESC) {
						am.MacroStop();
						break;
					}
				}
			}*/

			cout << "If you want to stop the macro \n";
			cout << "Select macro \n";

			HWND console = GetConsoleWindow();

			while (1) {
				//Sleep(1);
				if (GetForegroundWindow() == console) {
					am.MacroStop();

					cout << "Do you want to play macro? (yes/no) \n";
					cout << "INPUT : "; cin >> input;

					if (input.compare("Yes") == 0 || input.compare("yes") == 0) {
						cout << "Macro Play ... after 3 seconds\n";
						Sleep(3000);
						am.MacroPlay();
					}
					else {
						cout << "Do you want to start macro? (yes/no) \n";
						cout << "INPUT : "; cin >> input;
						if (input.compare("Yes") == 0 || input.compare("yes") == 0) {
							cout << "Do you want to random macro? (yes/no) \n";
							cout << "INPUT : "; cin >> input;
							if (input.compare("Yes") == 0 || input.compare("yes") == 0) {
								cout << "Macro start randomly ... after 3 seconds \n";
								Sleep(3000);
								am.MacroRandom();
							}
							else {
								cout << "Macro start ... after 3 seconds \n";
								Sleep(3000);
								am.MacroStart();
							}
						}
						else {
							cout << "Return the main. \n";
							break;
						}
					}
				}
			}

			break;
		}
		case '3': {
			cout << "===SAVE RECORDING DATA===" << endl;
			cout << "Please enter the file name" << endl;
			cout << "If you want to stop saving, enter 'END' " << endl;

			string input{};

			cout << "INPUT : ";  cin >> input; cout << endl;
			if (input.compare("END") == 0) {
				break;
			}

			input.append(".ini");

			if (rc.SaveRecordData(input)) {
				cout << "Success to save data" << endl;
			}
			else {
				cout << "Failed to save data" << endl;
			}
			break;
		}
		case '4': {
			cout << "===LOAD RECORDING DATA===" << endl;


			if (PrintFileLists("..\\Save")) {
				string input;
				do {
					cout << "Please enter the file name" << endl;
					cout << "If you wnat to stop loading, enter 'END" << endl;

					cout << "INPUT : "; cin >> input; cout << endl;
					if (input.compare("END") == 0) {
						break;
					}
					if (rc.LoadRecordData(input)) {
						cout << "Do you want to register your current algorithm? (Yes/No)" << endl;
						cout << "INPUT : "; cin >> input; cout << endl;

						if (input.compare("Yes") == 0) {
							am.RegisterCurrentAlgorithm();
						}

						auto rDatas = rc.GetRecordData(ctlVar++);

						am.Reset();
						for (auto& data : rDatas) {
							//2024-08-07 일단 첫 기록시 Enter 기록되는 거 로딩 막음
							if (data.recordingTime == 0 && data.vkCode == 13) continue;

							am.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode, data.scanCode);
						}

						if (!am.RegisterCurrentAlgorithm()) {
							ctlVar--;
							break;
						}
						am.MacroUpdate();
						am.MacroStop();
					}

					cout << "Do you want to continue loading? (Yes/No)" << endl;
					cout << "INPUT : "; cin >> input; cout << endl;
				} while (input.compare("Yes") == 0);

			}
			else {
				cout << "There are no files in that path." << endl;
			}
		}
				break;

		case '9':
			cout << "Closing..." << endl;
			goto EXIT;
			break;
		default:
			cout << "Invalid Value" << endl;
		}
		cout << endl << endl;
	}

EXIT:
	return 0;
}

bool PrintFileLists(const string path) {
	HANDLE fileControlHandle;
	vector<WIN32_FIND_DATAA> fileDatas{};
	WIN32_FIND_DATAA findFileData{};

	string addPath = path;
	addPath.append("\\*");

	fileControlHandle = FindFirstFileA(addPath.c_str(), &findFileData);
	if (fileControlHandle == NULL) {
		cerr << "Failed to FindFirstFileA" << endl;
		return false;
	}

	fileDatas.push_back(findFileData);

	while (FindNextFileA(fileControlHandle, &findFileData)) {
		fileDatas.push_back(findFileData);
	}

	FindClose(fileControlHandle);

	cout << "===FILE LISTS===" << endl;
	int idx = 0;

	for (auto& data : fileDatas) {
		unsigned int len = strlen(data.cFileName);

		const char* fileNameFilter = "ini.";
		const unsigned int filterLen = strlen(fileNameFilter);

		if (len < filterLen) {
			continue;
		}

		char* cmpFileNamePart = new char[filterLen];

		for (int idx = 0; idx < filterLen; idx++) {
			cmpFileNamePart[idx] = data.cFileName[len - 1 - idx];
		}

		if (strncmp(cmpFileNamePart, fileNameFilter, filterLen) == 0) {
			cout << ++idx << ". " << data.cFileName << endl;
		}

		delete(cmpFileNamePart);
	}

	return true;
}