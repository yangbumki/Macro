#pragma once

#include <iostream>
#include <string>

#include <Windows.h>

using namespace std;

typedef class DEACTIVATE_MACRO {
private:
	HWND procHwnd = NULL;

public:
	DEACTIVATE_MACRO();
	~DEACTIVATE_MACRO();

	bool GetWindow(const string title);
	bool GetWindow(const DWORD  pid);
}DeactivateMacro;