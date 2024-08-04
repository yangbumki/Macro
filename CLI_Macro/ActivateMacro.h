#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <Windows.h>

using namespace std;

typedef struct EXTENDED_INPUT {
	DWORD recordingTime;
	INPUT input;
}ExtendedInput;

typedef class ACTIVATE_MACRO {
public:
	//enum
	enum STATUS {
		MACRO_ERROR = -1,
		MACRO_INIT,
		MACRO_START,
		MACRO_STOP,
		MACRO_FINISH,
		MACRO_UPDATE
	};

private:
	//변수
	vector<vector<ExtendedInput>> datas;

	//vector<INPUT> inputs{};
	vector<ExtendedInput> inputs{};
	unsigned int tickTime;
	signed char macroStatus = MACRO_INIT;

	//스레드 관련 변수
	//다른 컴포넌트가 부착되었을 경우 스레드 간의 동기화를 위하여 미리 생성
	HANDLE  threadHandle  = NULL,
				 macroMtx = NULL;

	CRITICAL_SECTION	cs{};
	CONDITION_VARIABLE	updateConditionVar{};

	//함수
	void WarningMessage(const string msg);

	//2024-08-02 public 으로 이동
	/*bool SetMacroTime(int tickTime);
	unsigned int GetMacroTime();*/
	
	//스레드 관련 함수
	HANDLE GetMutex();
	//CONDITION_VARIABLE GetConditionVar();
	//CONDITION_VARIABLE& GetConditionVar();
	CONDITION_VARIABLE* GetConditionVar();
	CRITICAL_SECTION GetCriticalSection();

	bool CreateSlaveThread();
	static DWORD WINAPI MacroThread(LPVOID args);

public:
	ACTIVATE_MACRO(int tickTime = 0);
	~ACTIVATE_MACRO();

	byte GetMacroStatus();
	bool MacroStart();
	bool MacroStop();
	//스레드 동기화를 위한 함수
	bool MacroRun();
	bool MacroFinish();

protected:
	bool MacroUpdate();

public:
	bool SetMacroTime(int tickTime);
	unsigned int GetMacroTime();

	//bool RegisterMacroKey(const byte key);
	bool RegisterMacroKey(const byte key, const bool up = false);
	bool RegisterMacroKey(const DWORD time, const WPARAM keyType, const byte key,const DWORD scanCode);

	vector<EXTENDED_INPUT> GetRegisterInputs();
	
}ActivateMacro;