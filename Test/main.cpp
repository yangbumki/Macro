#include <iostream>
#include <Windows.h>

using namespace std;

int main(int argc, char* argv[]) {
	/*keybd_event(27, 1, WM_KEYDOWN, 0);
	keybd_event(27, 1, WM_KEYUP, 0);*/

	INPUT input1, input2;

	input1.type = 1;
	input1.ki.wVk = 27;
	input1.ki.wScan = 1;

	input1.type = 1;
	input1.ki.wVk = 27;
	input1.ki.wScan = 1;
	input1.ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(1, &input2, sizeof(INPUT));
	if (GetKeyState(27) != 0) {
		cout << "Success to get key state1 \n";
	}
	SendInput(1, &input1, sizeof(INPUT));
	if (GetKeyState(27) != 0) {
		cout << "Success to get key state2 \n";
	}
	
	

	return 0;
}