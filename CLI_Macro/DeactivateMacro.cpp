#include "DeactivateMacro.h"

DEACTIVATE_MACRO::DEACTIVATE_MACRO() {
}

DEACTIVATE_MACRO::~DEACTIVATE_MACRO() {
}

bool DEACTIVATE_MACRO::GetWindow(const string title) {
	auto hwnd = FindWindowA(NULL, title.c_str());
	if (hwnd == NULL) {
		cerr << "Failed to get window" << endl;
		return false;
	}

	this->procHwnd = hwnd;

	return true;
}

bool DEACTIVATE_MACRO::GetWindow(const DWORD pid) {
	HWND topHwnd = GetTopWindow(NULL);
	HWND curHwnd = topHwnd;

	DWORD curPid = 0;

	while (curHwnd) {
		GetWindowThreadProcessId(curHwnd, &curPid);
		if (curPid == pid) {
			this->procHwnd = curHwnd;
			return true;
		}
		 curHwnd = ::GetNextWindow(curHwnd, GW_HWNDNEXT);
	}

	cerr << "Failed to get window" << endl;

	return false;
}