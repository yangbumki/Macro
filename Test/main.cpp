#include <iostream>
#include <string>
#include <vector>

#include <Windows.h>
#include <conio.h>

using namespace std;

bool recordingStat = FALSE;

vector<RAWINPUT> rawinputs{};

LRESULT CALLBACK wndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INPUT: {
		// Raw Input 데이터 처리
		UINT dwSize;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			std::cerr << "GetRawInputData does not return correct size\n";
		}

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (recordingStat) {
			cout << "Recording" << endl;
			rawinputs.push_back(*raw);
		}

		if (raw->header.dwType == RIM_TYPEKEYBOARD) {
			RAWKEYBOARD rawKB = raw->data.keyboard;
			std::cout << "MakeCode: " << rawKB.MakeCode << ", Flags: " << rawKB.Flags
				<< ", Reserved: " << rawKB.Reserved << ", VKey: " << rawKB.VKey
				<< ", Message: " << rawKB.Message << std::endl;
		}

		delete[] lpb;
		return 0;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

DWORD WINAPI RawWndRegisterThreadFunc(void* arg) {
	WNDCLASS wc{};

	wc.lpfnWndProc = wndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"TEST";

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(0, L"TEST", L"TEST", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GetModuleHandle(NULL), NULL);
	if (hwnd == NULL) {
		cerr << "Failed to create window \n";
		return -1;
	}

	ShowWindow(hwnd, SW_SHOW);

	SetActiveWindow(hwnd);

	RAWINPUTDEVICE rid{};

	rid.usUsagePage = 0x01;
	rid.usUsage = 0x06;
	rid.dwFlags = 0;
	rid.hwndTarget = hwnd;

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
		cerr << "Failed to regitser raw input device \n";
		return -1;
	}

	MSG msg{};

	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

int main(int argc, char* argv[]) {
	HANDLE registerThread;


	registerThread = CreateThread(NULL, 0, RawWndRegisterThreadFunc, NULL, NULL, NULL);
	if (registerThread == NULL) {
		cerr << "Failed to create thread \n";
		return -1;
	}

	DWORD select;

	HWND maple = FindWindow(NULL, L"MapleStory");
	if (maple == NULL) {
		cerr << "Failed to find window \n";
		return 0;
	}

	while (1) {
		if (_kbhit()) {
			switch (_getch()) {
			case VK_RETURN:
				cout << "===RECORDING===\n";
				recordingStat = !recordingStat;
				break;
			case VK_TAB: {
				auto  lparamData = 0b000000000000000010000000000000001;
				for (auto& input : rawinputs) {
					SendMessage(maple, WM_ACTIVATEAPP, 1, 0);
					SendMessage(maple, WM_ACTIVATE, 1, 0);
					PostMessage(maple, WM_KEYDOWN, input.data.keyboard.VKey, lparamData);
					PostMessage(maple, WM_CHAR, 27, 0);
					PostMessage(maple, WM_INPUT, RIM_INPUT, (LPARAM)&input);
					PostMessage(maple, WM_KEYUP, input.data.keyboard.VKey, lparamData);
				}
				break;
			}
			case VK_ESCAPE:
				TerminateThread(registerThread, 0);
				WaitForSingleObject(registerThread, INFINITE);
				CloseHandle(registerThread);
				registerThread = NULL;
				goto END;
				break;
			default:
				break;
			}

		}
	}

END:
	return 0;
}
