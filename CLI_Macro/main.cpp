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
	vector<ActivateMacro> actMacros;
	unsigned int idx = 0;

	ActivateMacro tmpActMacro;
	Recorder rc;

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

			cout << "Mappling Recording Data" << endl;

			for (auto& data : rcDatas) {
				if (data.vkCode == VK_ESCAPE) break;
				if (!tmpActMacro.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode, data.scanCode)) {
					break;
				}
			}
			break;
		}
		case '2':
			if (actMacros.size() != 0) {
				int randomNum;
				actMacros[randomNum].MacroStart();
				cout << "Macro Start : after 3 seconds" << endl;
				Sleep(3000);

				actMacros[randomNum].MacroRun();
				cout << "If you want to end the macro" << endl;
				cout << "Press \"ESC\"" << endl;
				while (1) {
					actMacros[randomNum].GetMacroStatus();
					if (_kbhit()) {
						if (_getch() == ESC) {
							tmpActMacro.MacroStop();
							break;
						}
					}
				}
			}
			else {
				tmpActMacro.MacroStart();
				cout << "Macro Start : after 3 seconds" << endl;
				Sleep(3000);

				tmpActMacro.MacroRun();

				cout << "If you want to end the macro" << endl;
				cout << "Press \"ESC\"" << endl;
				while (1) {
					if (_kbhit()) {
						if (_getch() == ESC) {
							tmpActMacro.MacroStop();
							break;
						}
					}
				}
			}
			break;
		case '3': {
			cout << "===SAVE RECORDING DATA===" << endl;
			cout << "Please enter the file ntmpActMacroe" << endl;
			cout << "If you want to stop saving, enter 'END' " << endl;

			string fileNtmpActMacroe{};

			cout << "File NtmpActMacroe : ";  cin >> fileNtmpActMacroe; cout << endl;
			if (fileNtmpActMacroe.compare("END") == 0) {
				break;
			}

			fileNtmpActMacroe.append(".ini");

			if (rc.SaveRecordData(fileNtmpActMacroe)) {
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
					cout << "Please enter the file ntmpActMacroe" << endl;
					cout << "If you wnat to stop loading, enter 'END" << endl;

					cout << "INPUT : "; cin >> input; cout << endl;
					if (input.compare("END") == 0) {
						break;
					}

					if (rc.LoadRecordData(input)) {
						ActivateMacro am;
						auto rcDatas = rc.GetRecordData(idx);

						cout << "Mappling Recording Data" << endl;

						for (auto& data : rcDatas) {
							if (!am.RegisterMacroKey(data.recordingTime, data.keyType, data.vkCode, data.scanCode)) {
								break;
							}
						}

						actMacros.push_back(am);
					}
					
					cout << "Do you want to continue loading? (Yes/No)" << endl;
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