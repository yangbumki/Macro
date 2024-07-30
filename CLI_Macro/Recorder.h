#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <Windows.h>

using namespace std;

typedef struct KEYBOARD_HISTORY {
	DWORD recordingTime;
	WPARAM keyType;
	DWORD vkCode;
}KeyHistory;

typedef class RECORDER {
private:
	enum STATUS {
		REC_ERROR = -1,
		REC_INIT,
		REC_RUNNING,
		REC_STOP,
		REC_END
	};

	char status = REC_INIT;

	//해당 스레드는 무조건 하나만 컨트롤 할꺼라, 벡터 형태로 받지않음
	HANDLE threadHandle = NULL;

	static DWORD WINAPI RecordingThread(void* arg);
	static LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);

protected:
	static vector<KeyHistory> kbHistory;

public:
	RECORDER();
	~RECORDER();

	bool Recording();
	char GetStatus();
	bool Stop();
	void End();

	vector<KeyHistory> GetRecordData();
	bool ResetRecordData();
	bool SaveRecordData(const std::string fileName);

}Recorder;

