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
	//2024-07-31 Recorder ���� ���� �ݿ��ϱ� ���� 0 ���
	//if (tick <= 0) {
	//2024-08-02 �Է� �� ������ ���� ��� �� ��� ���� �߻�
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

//CONDITION_VARIABLE ACTIVATE_MACRO::GetConditionVar() {
//	return this->updateConditionVar;
//}

//CONDITION_VARIABLE& ACTIVATE_MACRO::GetConditionVar() {
//	return this->updateConditionVar;
//}

CONDITION_VARIABLE* ACTIVATE_MACRO::GetConditionVar() {
	return &this->updateConditionVar;
}

CRITICAL_SECTION ACTIVATE_MACRO::GetCriticalSection() {
	return cs;
}

bool ACTIVATE_MACRO::CreateSlaveThread() {
	if (threadHandle != NULL) {
		WarningMessage("Aleady exist thread haddle");
		return false;
	}

	//2024-07-22 ���ؽ� �߰�
	if (macroMtx != NULL) {
		WarningMessage("Aleady exist mutex");
		return false;
	}

	//���� ���� �߰�
	if (updateConditionVar.Ptr != nullptr) {
		WarningMessage("Aleady exist condition-variable");
		return false;
	}

	//�Ӱ迵�� �߰�
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

	auto updateCv = actMacro->GetConditionVar();

	CRITICAL_SECTION cs = actMacro->GetCriticalSection();

	vector<EXTENDED_INPUT> extInputs;
	int size = 0;

	unsigned int tickTime = actMacro->GetMacroTime();

	byte macroStat = 0;

	while (true) {
		macroStat = actMacro->GetMacroStatus();

		//�����̰� ���� ���, ��ũ�� ��ü �Է� �� �ߺ���
		if (tickTime > 0) {
				Sleep(tickTime);
		}
		else {
			//���ؽ�
			WaitForSingleObject(macroMtx, INFINITE);
			ReleaseMutex(macroMtx);
		}

		switch (macroStat) {
		case MACRO_STOP:
			while (actMacro->GetMacroStatus() != MACRO_START);
			break;
		case MACRO_INIT:
		case MACRO_ERROR:
			MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
			return -1;
		case MACRO_UPDATE:
			//SleepConditionVariableCS(&updateCv, &cs, INFINITE);
			//EnterCriticalSection(&cs);
			extInputs = actMacro->GetRegisterInputs();
			size = extInputs.size();

			//2024-08-02 ������Ʈ�� �پȵ� ���¿��� ��ũ�� ���۵Ǵ� ���·� ���� actMacro->MacroStart();
			//LeaveCriticalSection(&cs);
			//WakeConditionVariable(&updateCv);
			//WakeAllConditionVariable(&updateCv);
			// 2024-08-02 CONDITION_VARIABLE ���� ������ ���� ����ϴ� ���̾ƴ� �ּ� ���� ���
			//WakeConditionVariable(&actMacro->updateConditionVar);
			WakeConditionVariable(updateCv);
			break;
		case MACRO_START: {
			if (size < 0) {
				MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
				return -1;
			}

			/*for (int cnt = 0; cnt < size; cnt += 2) {
				SendInput(2, &inputs[cnt], sizeof(INPUT));
			}*/

			unsigned int prevRecordingTime = 0;

			for (auto& extInput : extInputs) {
				macroStat = actMacro->GetMacroStatus();
				if (macroStat == MACRO_STOP) break;

				Sleep(extInput.recordingTime - prevRecordingTime);
				prevRecordingTime = extInput.recordingTime;

				cout << "extInput.recordingTime : " << extInput.recordingTime << endl;
				SendInput(1, &extInput.input, sizeof(INPUT));
			}
			actMacro->MacroFinish();
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
	case MACRO_FINISH:
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

	if (threadHandle != NULL) {
		PostThreadMessage(GetThreadId(threadHandle), WM_QUIT, 1, 0);
	}

	return true;
}

bool ACTIVATE_MACRO::MacroRun() {
	if (macroStatus != MACRO_START) {
		WarningMessage("Not starting macro");
		return false;
	}

	ReleaseMutex(macroMtx);
	//2024-07-22 ���ؽ� ��ü ������ ��������
	WaitForSingleObject(macroMtx, INFINITE);
}

bool ACTIVATE_MACRO::MacroFinish() {
	this->macroStatus = MACRO_FINISH;

	return true;
}

bool ACTIVATE_MACRO::MacroUpdate() {
	EnterCriticalSection(&cs);
	this->macroStatus = MACRO_UPDATE;
	LeaveCriticalSection(&cs);
	
	//����ȭ
	//WakeConditionVariable(&updateConditionVar);
	SleepConditionVariableCS(&updateConditionVar, &cs, INFINITE);
	//cs�� ���� ����ؾ���
	return true;
}

//bool ACTIVATE_MACRO::RegisterMacroKey(const byte key) {
//	//key ����ó�� �ڵ� �߰� �ʿ�
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
	if (macroStatus == MACRO_INIT) {
		cerr << "Failed to register macro key" << endl;
		cerr << "Plz start macro" << endl;

		return false;
	}

	//key ����ó�� �ڵ� �߰� �ʿ�
	EXTENDED_INPUT extInput{};

	extInput.input.type = INPUT_KEYBOARD;
	extInput.input.ki.wVk = key;

	if (up)
		extInput.input.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs.push_back(extInput);

	MacroUpdate();

	return true;
}

bool ACTIVATE_MACRO::RegisterMacroKey(const DWORD time, const WPARAM keyType, const byte key, const DWORD scanCode) {
	if (macroStatus == MACRO_INIT) {
		cerr << "Failed to register macro key" << endl;
		cerr << "Plz start macro" << endl;

		return false;
	}

	EXTENDED_INPUT extInput{};

	if (time < 0) {
		cerr << "Failed to register macro key" << endl;
		return false;
	}

	extInput.recordingTime = time;
	extInput.input.type = INPUT_KEYBOARD;
	extInput.input.ki.wVk = key;
	extInput.input.ki.wScan = scanCode;

	if (keyType == WM_KEYUP)
		extInput.input.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs.push_back(extInput);

	/*datas.push_back(inputs);*/

	MacroUpdate();

	return true;
}

vector<EXTENDED_INPUT> ACTIVATE_MACRO::GetRegisterInputs() {
	if (inputs.empty()) {
		WarningMessage("Nothings exist Inputs");
	}

	return this->inputs;
}