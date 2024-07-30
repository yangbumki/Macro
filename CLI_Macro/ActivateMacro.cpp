#include "ActivateMacro.h"

ACTIVATE_MACRO::ACTIVATE_MACRO(int tickTime) {
	SetMacroTime(tickTime);

	this->MacroStart();
}

ACTIVATE_MACRO::~ACTIVATE_MACRO() {

}

void ACTIVATE_MACRO::WarningMessage(const string msg) {
	cout << "===[ACTIVATE_MACRO]===" << endl;
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

HANDLE ACTIVATE_MACRO::GetMutex() {
	if (macroMtx == NULL) {
		WarningMessage("Failed to GetMutex");
	}

	return macroMtx;
}

CONDITION_VARIABLE ACTIVATE_MACRO::GetConditionVar() {
	return this->updateConditionVar;
}

CRITICAL_SECTION ACTIVATE_MACRO::GetCriticalSection() {
	return cs;
}

bool ACTIVATE_MACRO::CreateSlaveThread() {
	if (threadHandle != NULL) {
		WarningMessage("Aleady exist thread haddle");
		return false;
	}

	//2024-07-22 뮤텍스 추가
	if (macroMtx != NULL) {
		WarningMessage("Aleady exist mutex");
		return false;
	}

	//조건 변수 추가
	if (updateConditionVar.Ptr != nullptr) {
		WarningMessage("Aleady exist condition-variable");
		return false;
	}

	//임계영역 추가
	if (cs.DebugInfo != NULL) {
		WarningMessage("Aleady exist critical-section");
		return false;
	}

	macroMtx = CreateMutex(NULL, TRUE, NULL);
	if (macroMtx == NULL) {
		WarningMessage("Failed to createMutex");
		return false;
	}

	InitializeConditionVariable(&updateConditionVar);
	
	InitializeCriticalSection(&cs);

	threadHandle = CreateThread(NULL, 0, MacroThread, this, 0, NULL);
	if (threadHandle == NULL) {
		WarningMessage("Failed to create thread");
		return false;
	}

	return true;
}

DWORD WINAPI ACTIVATE_MACRO::MacroThread(LPVOID args) {
	ActivateMacro* actMacro = (ACTIVATE_MACRO*)args;
	if (actMacro == nullptr || actMacro == NULL) {
		MessageBox(NULL, L"Failed to start thread", L"ERROR", NULL);
		return -1;
	}

	HANDLE macroMtx = actMacro->GetMutex();
	if (macroMtx == NULL) {
		MessageBox(NULL, L"Failed to GetMutex", L"ERROR", NULL);
		return -1;
	}

	CONDITION_VARIABLE updateCv = actMacro->GetConditionVar();

	CRITICAL_SECTION cs = actMacro->GetCriticalSection();

	vector<EXTENDED_INPUT> extInputs;
	int size = 0;

	unsigned int tickTime = actMacro->GetMacroTime();

	while (true) {
		if (tickTime > 0) {
			Sleep(tickTime);
		}
		else {
			//뮤텍스
			WaitForSingleObject(macroMtx, INFINITE);
			ReleaseMutex(macroMtx);
		}

		switch (actMacro->GetMacroStatus()) {
		case MACRO_STOP:
			while (actMacro->GetMacroStatus() != MACRO_START);
			break;
		case MACRO_INIT:
		case MACRO_ERROR:
			MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
			return -1;
		case MACRO_UPDATE:
			//SleepConditionVariableCS(&updateCv, &cs, INFINITE);
			extInputs = actMacro->GetRegisterInputs();
			size = extInputs.size();

			actMacro->MacroStart();
			WakeConditionVariable(&updateCv);
			break;
		case MACRO_START:
			if (size < 0) {
				MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
				return -1;
			}

			/*for (int cnt = 0; cnt < size; cnt += 2) {
				SendInput(2, &inputs[cnt], sizeof(INPUT));
			}*/

			for (auto& extInput : extInputs) {
				Sleep(extInput.recordingTime);
				SendInput(1, &extInput.input, sizeof(INPUT));
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

bool ACTIVATE_MACRO::MacroRun() {
	if (macroStatus != MACRO_START) {
		WarningMessage("Not starting macro");
		return false;
	}

	ReleaseMutex(macroMtx);
	//2024-07-22 뮤텍스 개체 소유권 가져오기
	WaitForSingleObject(macroMtx, INFINITE);
}

bool ACTIVATE_MACRO::MacroUpdate() {
	EnterCriticalSection(&cs);
	this->macroStatus = MACRO_UPDATE;
	LeaveCriticalSection(&cs);
	//동기화
	//WakeConditionVariable(&updateConditionVar);
	SleepConditionVariableCS(&updateConditionVar, &cs, INFINITE);
	//cs랑 같이 사용해야함
	return true;
}

//bool ACTIVATE_MACRO::RegisterMacroKey(const byte key) {
//	//key 예외처리 코드 추가 필요
//
//	INPUT inputDown, inputUp;
//
//	inputDown.type = INPUT_KEYBOARD;
//	inputDown.ki.wVk = key;
//
//	inputUp.type = INPUT_KEYBOARD;
//	inputUp.ki.wVk = key;
//	inputUp.ki.dwFlags = KEYEVENTF_KEYUP;
//
//	inputs.push_back(inputDown);
//	inputs.push_back(inputUp);
//
//	MacroUpdate();
//
//	return true;
//}

bool ACTIVATE_MACRO::RegisterMacroKey(const byte key, const bool up) {
	//key 예외처리 코드 추가 필요
	EXTENDED_INPUT extInput{};

	extInput.input.type = INPUT_KEYBOARD;
	extInput.input.ki.wVk = key;

	if (up)
		extInput.input.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs.push_back(extInput);

	MacroUpdate();

	return true;
}

bool ACTIVATE_MACRO::RegisterMacroKey(const DWORD time, const WPARAM keyType, const byte key) {
	EXTENDED_INPUT extInput{};

	if (time < 0) {
		cerr << "Failed to register macro key" << endl;
		return false;
	}

	extInput.recordingTime = time;
	extInput.input.type = INPUT_KEYBOARD;
	extInput.input.ki.wVk = key;

	if (keyType == WM_KEYUP)
		extInput.input.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs.push_back(extInput);

	MacroUpdate();

	return true;
}

vector<EXTENDED_INPUT> ACTIVATE_MACRO::GetRegisterInputs() {
	if (inputs.empty()) {
		WarningMessage("Nothings exist Inputs");
	}

	return this->inputs;
}