#include <iostream>
#include <vector>

#include <Windows.h>

using namespace std;
//
//HHOOK hook;
//
//LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
//	cout << nCode << endl;
//	cout << wParam << endl;
//	cout << lParam<< endl;
//
//	KBDLLHOOKSTRUCT* kbs;
//	
//	kbs = (KBDLLHOOKSTRUCT*)lParam;
//
//	return CallNextHookEx(NULL, nCode, wParam, lParam);
//}
//
//int main(int argc, char* argv[], char* env[]) {
//	hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, NULL);
//	if (hook == NULL) {
//		cerr << "Failed to call SetWindowsHookEx" << endl;
//		return -1;
//	}
//	MSG msg;
//
//	while (GetMessage(&msg, NULL, 0, 0)) {
//		TranslateMessage(&msg);
//		DispatchMessageW(&msg);
//	}
//	
//	return 0;
//}

HWND procHwnd;

bool GetWindow(const DWORD pid) {
	HWND topHwnd = GetTopWindow(NULL);
	HWND curHwnd = topHwnd;

	DWORD curPid = 0;

	while (curHwnd) {
		GetWindowThreadProcessId(curHwnd, &curPid);
		if (curPid == pid) {
			procHwnd = curHwnd;
			return true;
		}
		curHwnd = ::GetNextWindow(curHwnd, GW_HWNDNEXT);
	}

	cerr << "Failed to get window" << endl;

	return false;
}

int main() {
	HWND hwnd;

	hwnd = FindWindowA(NULL, "MapleStory");
	if (hwnd == NULL) {
		cerr << "Failed to FindWindowA" << endl;
		return -1;
	}

	

	RECT rect{};
	GetWindowRect(hwnd, &rect);

	UINT deviceCnt;
	RAWINPUTDEVICELIST* deviceLists = nullptr;

	if (GetRawInputDeviceList(NULL, &deviceCnt, sizeof(RAWINPUTDEVICELIST)) == 0) {
		deviceLists = new RAWINPUTDEVICELIST[deviceCnt];

		if (GetRawInputDeviceList(deviceLists, &deviceCnt, sizeof(RAWINPUTDEVICELIST)) == -1) {
			cerr << "Failed to GetRawInputDeviceList" << endl;
		}
	}

	cout << "===DEVICE LISTS===" << endl;
	for (int idx = 0; idx < deviceCnt; idx++) {
		cout << "TYPE : " << deviceLists[idx].dwType << endl;
		cout << "HANDLE : " << deviceLists[idx].hDevice << endl;

		char buf[MAX_PATH] = {0, };
		UINT bufSize = MAX_PATH;

		if (deviceLists[idx].dwType == RIM_TYPEKEYBOARD) {
			if (GetRawInputDeviceInfo(deviceLists[idx].hDevice, RIDI_DEVICENAME, buf, &bufSize) != -1) {
				for (int i = 0; i < bufSize; i++) {
					cout << buf[i];
				}
				RAWINPUT ri;
				GetRawInputData(

				cout << endl;
			}
		}
	}

	std::cout << endl << endl;

	

	RAWINPUT rawInput;

	rawInput.header.dwType = RIM_TYPEKEYBOARD;
	rawInput.data.keyboard.VKey = VK_ESCAPE;

	while (1) {
		Sleep(1000);

		EnableWindow(hwnd, true);
		SetActiveWindow(hwnd);
		SetFocus(hwnd);
		
		DWORD lParam = 0b00000000000000010000000000000001;

		SendMessage(hwnd, WM_INPUT, RIM_INPUT, 1);
		SendMessage(hwnd, WM_WINDOWPOSCHANGING, NULL, 0x0014EAD0);
		SendMessage(hwnd, WM_WINDOWPOSCHANGED, NULL, 0x0014EAD0);
		SendMessage(hwnd, WM_ACTIVATEAPP, true, 0);
		SendMessage(hwnd, WM_ACTIVATE, WA_CLICKACTIVE, 0);
		SendMessage(hwnd, WM_IME_SETCONTEXT, TRUE, 0xC000000F);
		SendMessage(hwnd, WM_IME_NOTIFY, IMN_OPENSTATUSWINDOW, 2);
		SendMessage(hwnd, WM_SETFOCUS, FALSE, 0);
		
		PostMessage(hwnd, WM_KEYDOWN, VK_ESCAPE, lParam);
		//PostMessage(hwnd, WM_KEYUP, VK_ESCAPE, lParam);
	}

	return 0;
}