#include "ActivateMacro.h"

ACTIVATE_MACRO::ACTIVATE_MACRO(int tickTime) {
	SetMacroTime(tickTime);

	this->MacroStart();
	//2024-08-08 매크로 스레드 돌아감 방지
	this->MacroStop();
}

ACTIVATE_MACRO::~ACTIVATE_MACRO() {
	for (auto& algorithm : macroAlgorithms) {
		algorithm.clear();
	}

	macroAlgorithms.clear();

	Reset();

	CloseHandle(threadHandle);
	CloseHandle(macroMtx);
}

void ACTIVATE_MACRO::WarningMessage(const string msg) {
	cout << endl;
	cout << "===[ACTIVATE_MACRO]===" << endl;
	cerr << "[WARNING] : " << msg << endl;
}

bool ACTIVATE_MACRO::SetMacroTime(int tick) {
	//2024-07-31 Recorder 값을 고대로 반영하기 위해 0 허용
	//if (tick <= 0) {
	//2024-08-02 입력 값 딜레이 없이 사용 할 경우 오류 발생
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

	auto updateCv = actMacro->GetConditionVar();

	CRITICAL_SECTION cs = actMacro->GetCriticalSection();

	vector<EXTENDED_INPUT> extInputs;
	int size = 0;

	unsigned int tickTime = actMacro->GetMacroTime();

	//2024-08-19 byte -> int
	//byte macroStat = 0;
	int macroStat = 0;

	//랜덤 변수
	random_device rd;
	default_random_engine engine(rd());
	uniform_int_distribution<int> recordingRandomTime(DURATION_MIN, DURATION_MAX);

	while (true) {
		macroStat = actMacro->GetMacroStatus();

		//딜레이가 없을 경우, 매크로 자체 입력 값 중복됨
		if (tickTime > 0) {

			//2024-08-05 대충 추가 삭제해야함
			static int rn;
			if (rn != 0)
				Sleep(tickTime * rn);
			else
				Sleep(tickTime);
		}
		else {
			//뮤텍스
			WaitForSingleObject(macroMtx, INFINITE);
			ReleaseMutex(macroMtx);
		}

		switch (macroStat) {
		case MACRO_STOP:
			//2024-08-19 stop 알고리즘 변경
			/*while (actMacro->GetMacroStatus() != MACRO_START && actMacro->GetMacroStatus() != MACRO_UPDATE);*/
			while (actMacro->GetMacroStatus() == MACRO_STOP) Sleep(1);
			break;
		case MACRO_INIT:
		case MACRO_ERROR:
			MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
			return -1;
		case MACRO_UPDATE:
			UPDATE:
			//SleepConditionVariableCS(&updateCv, &cs, INFINITE);
			//EnterCriticalSection(&cs);
			extInputs = actMacro->GetRegisterInputs();
			size = extInputs.size();

			//2024-08-02 업데이트가 다안된 상태에서 매크로 시작되는 상태로 변함 actMacro->MacroStart();
			//LeaveCriticalSection(&cs);
			//WakeConditionVariable(&updateCv);
			//WakeAllConditionVariable(&updateCv);
			// 2024-08-02 CONDITION_VARIABLE 내부 포인터 값을 사용하는 것이아닌 주소 값을 사용
			//WakeConditionVariable(&actMacro->updateConditionVar);
			WakeConditionVariable(updateCv);
			break;
		case MACRO_RANDOM:
		case MACRO_START: {
			if (size < 0) {
				MessageBox(NULL, L"Failed to running thread", L"ERROR", NULL);
				return -1;
			}

			/*for (int cnt = 0; cnt < size; cnt += 2) {
				SendInput(2, &inputs[cnt], sizeof(INPUT));
			}*/

			//2024-08-19 macro play 변수 추가
			int prevMacroStat = macroStat;

			unsigned int randomDuration = 0;
			
			for (auto& extInput : extInputs) {
				macroStat = actMacro->GetMacroStatus();
				//2024-08-19 스탑 알고리즘 변경
				/*if (macroStat == MACRO_STOP) break;*/
				if (macroStat == MACRO_STOP) {
					//Sleep(1) 중요 없으면 릴리즈 동작 x
					while ((macroStat = actMacro->GetMacroStatus()) == MACRO_STOP) Sleep(1);
					
					if (macroStat != MACRO_PLAY) break;

					actMacro->macroStatus = macroStat =  prevMacroStat;
				}

				//2024-08-06 로직변경
				//Sleep(extInput.recordingTime - prevRecordingTime);
				//2024-08-10 랜덤 값 적용
				randomDuration = recordingRandomTime(engine);
				cout << "extInput.recordingTime : " << extInput.recordingTime << " randomDuration : " << randomDuration << endl;

				//2의 배수일 경우
				if (randomDuration % 2 == 0) {
					//마이너스
					extInput.recordingTime <= randomDuration ? Sleep(randomDuration) : Sleep(extInput.recordingTime - randomDuration);
				}
				else {
					//플러스
					Sleep(extInput.recordingTime + randomDuration);
				}


				//cout << "extInput.recordingTime : " << extInput.recordingTime << endl;
				
				
				//2024-08-07 차이 확인
				/*SendInput(1, &extInput.input, sizeof(INPUT));*/
				keybd_event(extInput.input.ki.wVk, extInput.input.ki.wScan, extInput.input.ki.dwFlags, 0);
				/*auto result = GetAsyncKeyState(extInput.input.ki.dwFlags);
				cout << "Result : " << result << endl;*/

				//do {
				//	/*SendInput(1, &extInput.input, sizeof(INPUT));*/
				//	keybd_event(extInput.input.ki.wVk, extInput.input.ki.wScan, extInput.input.ki.dwFlags, 0);
				//}
				//while (!GetAsyncKeyState(extInput.input.ki.wVk));
			}

			if (macroStat == MACRO_RANDOM) {
				//2024-08-10 한번 지정된 최댓 값 변경 불가로 매번생성
				uniform_int_distribution<int> marcoAlgorithm(0, actMacro->GetAlgorithmCount());
				//임시
				static int rn;
				static int old;

				//2024-08-08 랜덤 알고리즘 수정
				do {
					rn = marcoAlgorithm(engine);
					if (old != rn) {
						old = rn;
						break;
					}

				} while (1);

				cout << "===CHANGE ALGORITHM===\n";
				cout << "Random Number : " << rn << endl;

				actMacro->SelectAlgorithm(rn);
				//임시 방편..
				goto UPDATE;
			}

			break;
		}
		default:
			cout << "===ACTIVATE MACRO THREAD === \n";
			cout << "Macro status is starnge \n";
			break;
		}
	}

	return 0;
}

bool ACTIVATE_MACRO::Reset() {
	if (inputs.size() <= 0) {
		cerr << "Aleady clear macro" << endl;
		return false;
	}
	
	this->inputs.clear();
	inputs.resize(0);

	return true;
}

//byte ACTIVATE_MACRO::GetMacroStatus() {
int ACTIVATE_MACRO::GetMacroStatus() {
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

}

bool ACTIVATE_MACRO::MacroStop() {
	switch (GetMacroStatus()) {
	case MACRO_ERROR:
	case MACRO_INIT:
	case MACRO_STOP:
		WarningMessage("Failed to macro stop");
		return false;
	case MACRO_RANDOM:
	case MACRO_UPDATE:
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

bool ACTIVATE_MACRO::MacroPlay() {
	// 2024-08-19 기존의 방식이 아닌 매크로 스탯 자체를 변경하는 방식으로 변경
	macroStatus = MACRO_PLAY;

	return true;
}

//2024-08-04 지금 안씀
//bool ACTIVATE_MACRO::MacroRun() {
//	if (macroStatus != MACRO_START) {
//		WarningMessage("Not starting macro");
//		return false;
//	}
//
//	ReleaseMutex(macroMtx);
//	//2024-07-22 뮤텍스 개체 소유권 가져오기
//	WaitForSingleObject(macroMtx, INFINITE);
//}

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

bool ACTIVATE_MACRO::MacroRandom() {
	switch (GetMacroStatus()) {
	case MACRO_ERROR:
	case MACRO_RANDOM:
		WarningMessage("Failed to random macro");
		return false;
		break;
	case MACRO_INIT:
	case MACRO_START:
	case MACRO_STOP:
	case MACRO_UPDATE:
		macroStatus = MACRO_RANDOM;
		break;
	default:
		WarningMessage("Failed to random macro");
		return false;
		break;
	}

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

//bool ACTIVATE_MACRO::RegisterMacroKey(const byte key, const bool up) {
bool ACTIVATE_MACRO::RegisterMacroKey(const WORD key, const bool up) {
	if (macroStatus == MACRO_INIT) {
		cerr << "Failed to register macro key" << endl;
		cerr << "Plz start macro" << endl;

		return false;
	}

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

//bool ACTIVATE_MACRO::RegisterMacroKey(const DWORD time, const WPARAM keyType, const byte key, const DWORD scanCode) {
bool ACTIVATE_MACRO::RegisterMacroKey(const DWORD time, const WPARAM keyType, const WORD key, const DWORD scanCode) {
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

	//2024-08-04 자동 매크로 업데이트는 좋으나, 성능 문제로 인해 일단 보류
	//MacroUpdate();

	return true;
}

bool ACTIVATE_MACRO::RegisterCurrentAlgorithm() {
	if (inputs.empty()) {
		cerr << "Failed to register current algorithm" << endl;
		return false;
	}

	macroAlgorithms.push_back(inputs);

	return true;
}

byte ACTIVATE_MACRO::GetAlgorithmCount() {
	return macroAlgorithms.size()-1;
}

bool ACTIVATE_MACRO::SelectAlgorithm(int idx) {
	//어차피 사이즈 체크 들어가기 때문에 empty 체크 필요 없음
	/*if (macroAlgorithms.empty()) {
		cerr << "Failed to select algorithm" << endl;
		return false;
	}
	else */
	if (idx < 0 || idx > GetAlgorithmCount()) {
		cerr<<"Failed to select Algorithm" << endl;
		return false;
	}

	inputs = macroAlgorithms[idx];
	
	return true;
}

vector<EXTENDED_INPUT> ACTIVATE_MACRO::GetRegisterInputs() {
	if (inputs.empty()) {
		WarningMessage("Nothings exist Inputs");
	}

	return this->inputs;
}