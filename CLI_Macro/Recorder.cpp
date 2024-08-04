#include "Recorder.h"

RECORDER::RECORDER() {
	status = REC_INIT;
}

RECORDER::~RECORDER() {
	End();
	//스레드 종료
	WaitForSingleObject(threadHandle, INFINITE);
	CloseHandle(threadHandle);
}

DWORD WINAPI RECORDER::RecordingThread(void* arg) {
	Recorder* rc = (Recorder*)arg;
	
	if (rc == nullptr || rc == NULL) {
		//msgbox 이용 방식 사용하였으나, 각 클래스의 에러 메소드 활용 가능성 있음
		MessageBoxA(NULL, "Failed to create recroding thread", "ERROR", NULL);
		return -1;
	}

	static vector<KeyHistory> kbHistory{};
	HHOOK hookHandle = NULL;

	hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, rc->KeyHookProc, NULL, NULL);
	if (hookHandle == NULL) {
		MessageBoxA(NULL, "Failed to SetWindowsHookEx", "ERROR", NULL);
		return -1;
	}

	MSG msg;
	char status;

	while (status = rc->GetStatus()) {
		if (status != REC_RUNNING) {
			if (status == REC_STOP) {
				// 무한대기
				while (status != REC_RUNNING) {
					status = rc->GetStatus();
				}
			}
			else if (status == REC_END) {
				//스레드 종료
				break;
			}
		}

		else {
			if (GetMessage(&msg, 0, NULL, NULL) != 0) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}

		}
	}

	UnhookWindowsHookEx(hookHandle);

	return 0;
}

vector<KeyHistory> RECORDER::kbHistory;

LRESULT CALLBACK RECORDER::KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	//wParam 가상 키 코드
	//cout << "wParam : " << wParam << endl;

	if (nCode < 0) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	
	KBDLLHOOKSTRUCT* kls = (KBDLLHOOKSTRUCT*)lParam;

	//GetMessageTime() 함수 동일
	static LONG time = kls->time;
	unsigned int recordingTime = kls->time - time;

	//cout << "Recording Time " << (kls->time - time) / 1000 << "." << (kls->time - time) % 1000 << endl;

 	cout << "wParam : " << wParam << endl;
	cout << "lParam : " << lParam<< endl;
	cout << "kls->vkCode : " << kls->vkCode << endl;
	cout << "kls->scanCode : " << kls->scanCode << endl;
	
	KeyHistory kh{};

	kh.keyType = wParam;
	kh.recordingTime = recordingTime;
	kh.vkCode = kls->vkCode;
	kh.scanCode = kls->scanCode;
	
	
	kbHistory.push_back(kh);

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool RECORDER::Recording() {
	if (threadHandle == NULL) {
		threadHandle = CreateThread(NULL, 0, RecordingThread, this, NULL, NULL);
		if (threadHandle == NULL) {
			cerr << "Failed to create thread" << endl;
			return false;
		}
	}

	status = REC_RUNNING;

	return true;
}

char RECORDER::GetStatus() {
	return this->status;
}

bool RECORDER::Stop() {
	status = REC_STOP;

	if (threadHandle != NULL) {
		PostThreadMessage(GetThreadId(threadHandle), WM_QUIT, 1, 0);
	}
	
	return true;
}

void RECORDER::End() {
	status = REC_END;

	if (threadHandle != NULL) {
		PostThreadMessage(GetThreadId(threadHandle), WM_QUIT, 1, 0);
	}
}

vector<KeyHistory> RECORDER::GetRecordData(int idx) {
	return idx == -1 ? kbHistory : this->datas[idx];
}

bool RECORDER::ResetRecordData() {
	if (kbHistory.empty()) {
		cerr << "Aleady empty record data" << endl;
		return false;
	}

	kbHistory.clear();
	kbHistory.resize(0);

	return true;
}

bool RECORDER::SaveRecordData(const string fileName) {
	string path = "..\\Save\\";
	path.append(fileName);
	
	if (kbHistory.empty()) {
		cerr << "Failed to save record data" << endl;
		return false;
	}

	int idx = 0;

	for (auto& kb : kbHistory) {
		auto order = to_string(++idx);

		if (WritePrivateProfileStringA(order.c_str(), "RecordingTime", to_string(kb.recordingTime).c_str(), path.c_str())) {

			if (kb.keyType == WM_KEYDOWN) {
				WritePrivateProfileStringA(order.c_str(), "KeyType", "KEYDOWN", path.c_str());
			}
			else if (kb.keyType == WM_KEYUP) {
				WritePrivateProfileStringA(order.c_str(), "KeyType", "KEYUP", path.c_str());
			}

			WritePrivateProfileStringA(order.c_str(), "VKCode", to_string(kb.vkCode).c_str(), path.c_str());

			WritePrivateProfileStringA(order.c_str(), "ScanCode", to_string(kb.scanCode).c_str(), path.c_str());
		} else {
			return false;
		}
	}

	return true;
}

bool RECORDER::LoadRecordData(const string fileName) {
	string path = "..\\Save\\";

	path.append(fileName);

	//unsigned int size = sizeof(unsigned short);
	unsigned int size = 1024;

	char* sectionNames = new char[size] {};
	int recvBytes = 0;

	recvBytes = GetPrivateProfileSectionNamesA(sectionNames, size, path.c_str());
	if (recvBytes <= 0) {
		cerr << "Failed to load record data" << endl;
		return false;
	}

	unsigned int idx;

	for (idx = recvBytes; idx > 0; idx--) {
		if (sectionNames[idx-1] = '\0') {
			break;
		}
	}

	unsigned int maxIdx = atoi(&sectionNames[idx]);

	delete(sectionNames);

	vector<KeyHistory> tmpKeyHistory;
	KeyHistory kh;

	const char* DEFAULT = "NONE";
	char retString[MAX_PATH] = { 0, };

	for (idx = 0; idx < maxIdx; idx++) {
		//2024-08-04
		//저장 해야될 값이 많아질 경우, 하나의 함수로 묶어서 사용 해야함
		//Key 값 또한 하나의 구조체와 맵핑된 enum 값을 사용하면 편리

		ZeroMemory(&kh, sizeof(KeyHistory));

		auto order = to_string(++idx);

		if (GetPrivateProfileStringA(order.c_str(), "RecordingTime", DEFAULT, retString, MAX_PATH, path.c_str()) <= 0) {
			cerr << "Failed to load record data" << endl;
			return false;
		}

		kh.recordingTime = atoi(retString);

		memset(retString, 0, MAX_PATH);

		if (GetPrivateProfileStringA(order.c_str(), "KeyType", DEFAULT, retString, MAX_PATH, path.c_str()) <= 0) {
			cerr << "Failed to load record data" << endl;
			return false;
		}


		kh.keyType = atoi(retString);

		memset(retString, 0, MAX_PATH);

		if (GetPrivateProfileStringA(order.c_str(), "VKCode", DEFAULT, retString, MAX_PATH, path.c_str()) <= 0) {
			cerr << "Failed to load record data" << endl;
			return false;
		}

		kh.vkCode = atoi(retString);

		memset(retString, 0, MAX_PATH);

		if (GetPrivateProfileStringA(order.c_str(), "ScanCode", DEFAULT, retString, MAX_PATH, path.c_str()) <= 0) {
			cerr << "Failed to load record data" << endl;
			return false;
		}

		kh.scanCode = atoi(retString);
		memset(retString, 0, MAX_PATH);

		tmpKeyHistory.push_back(kh);
	}

	this->datas.push_back(tmpKeyHistory);

	return true;
}