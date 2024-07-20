#include "ActivateMacro.h"

ACTIVATE_MACRO::ACTIVATE_MACRO(int tickTime) {
	SetMacroTime(tickTime);

}

ACTIVATE_MACRO::~ACTIVATE_MACRO() {

}

void ACTIVATE_MACRO::WarningMessage(const string msg) {
	cout<< "===[ACTIVATE_MACRO]===" << endl;
	cerr << "[WARNING] : " << msg << endl;
}



bool ACTIVATE_MACRO::SetMacroTime(int tick) {
	if (tick <= 0) {
		WarningMessage("Failed to InitMacroTime");
		return false;
	}

	this->tickTime = tick;
	return true;
}

unsigned int ACTIVATE_MACRO::GetMacroTime() {
	if (this->tickTime <= 0) {
		WarningMessage("Failed to GetMacroTime");
		return -1;
	}
	return this->tickTime;
}

bool ACTIVATE_MACRO::CreateSlaveThread() {
	if(threadHandle == NULL) {
		WarningMessage("Aleady exist thread haddle");
		return false;
	}

	threadHandle = CreateThread(NULL, 0, MacroThread, this, 0, NULL);
	if (threadHandle == NULL) {
		WarningMessage("Failed to create thread");
		return false;
	}
}

DWORD WINAPI ACTIVATE_MACRO::MacroThread(LPVOID args) {
	ActivateMacro* actMacro = (ACTIVATE_MACRO*)args;
	if (actMacro == nullptr || actMacro == NULL) {
		MessageBox(NULL, L"Failed to start thread", L"ERROR", NULL);
		return -1;
	}

	vector<INPUT> inputs;
	int size = 0;

	while (true) {
		switch (actMacro->GetMacroStatus()) {
		case MACRO_STOP:
			while (actMacro->GetMacroStatus() != MACRO_START);
			break;
		case MACRO_INIT:
		case MACRO_ERROR:
			MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
			return -1;
		case MACRO_UPDATE:
			inputs = actMacro->GetRegisterInputs();
			size = inputs.size();

			actMacro->MacroStart();
			break;
		case MACRO_START:
			if (size <= 0) {
				MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
				return -1;
			}
			for (int cnt = 0; cnt < size; cnt+=2) {
				SendInput(2, &inputs[cnt], sizeof(INPUT));
			}
		default:
			break;
		}
	}

	return 0;
}

byte ACTIVATE_MACRO::GetMacroStatus() {
	return this->macroStatus;
}

bool ACTIVATE_MACRO::MacroStart() {
	switch (GetMacroStatus()) {
	case MACRO_ERROR:
	case MACRO_START:
		WarningMessage("Failed to macro start");
		return false;
	case MACRO_INIT:
		if (!CreateSlaveThread()) {
			WarningMessage("Failed to mactro start");
			return false;
		}
	case MACRO_UPDATE:
	case MACRO_STOP:
		macroStatus = MACRO_START;
		break;
	default:
		break;
	}

	return true;
}

bool ACTIVATE_MACRO::MacroStop() {
	switch (GetMacroStatus()) {
	case MACRO_ERROR:
	case MACRO_INIT:
	case MACRO_STOP:
		WarningMessage("Failed to macro stop");
		return false;
	case MACRO_START:
		macroStatus = MACRO_STOP;
		break;
	default:
		break;
	}

	return true;
}

bool ACTIVATE_MACRO::MacroUpdate() {
	this->macroStatus = MACRO_UPDATE;
	return true;
}

bool ACTIVATE_MACRO::RegisterMacroKey(const byte key) {
	//key 예외처리 코드 추가 필요

	INPUT inputDown, inputUp;

	inputDown.type = INPUT_KEYBOARD;
	inputDown.ki.wVk = key;

	inputUp.type = INPUT_KEYBOARD;
	inputUp.ki.wVk = key;
	inputUp.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs.push_back(inputDown);
	inputs.push_back(inputUp);

	MacroUpdate();

	return true;
}

vector<INPUT> ACTIVATE_MACRO::GetRegisterInputs() {
	if (inputs.empty()) {
		WarningMessage("Nothings exist Inputs");
	}

	return this->inputs;
}