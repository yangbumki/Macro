#include <iostream>

#include <Windows.h>

using namespace std;

HHOOK hook;

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	cout << nCode << endl;
	cout << wParam << endl;
	cout << lParam<< endl;

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main(int argc, char* argv[], char* env[]) {
	hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, NULL);
	if (hook == NULL) {
		cerr << "Failed to call SetWindowsHookEx" << endl;
		return -1;
	}
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	
	return 0;
}