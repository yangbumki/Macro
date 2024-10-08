#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <random>

#include <Windows.h>

using namespace std;

#define DURATION_MIN	5
#define DURATION_MAX	10

typedef struct EXTENDED_INPUT {
	DWORD recordingTime;
	INPUT input;
}ExtendedInput;

typedef class ACTIVATE_MACRO {
private:
	//enum
	enum STATUS {
		MACRO_ERROR = -1,
		MACRO_INIT,
		MACRO_START,
		MACRO_STOP,
		MACRO_PLAY,
		MACRO_UPDATE,
		MACRO_RANDOM,
	};

	//변수
	vector<vector<ExtendedInput>> macroAlgorithms;

	//vector<INPUT> inputs{};
	vector<ExtendedInput> inputs{};

	unsigned int tickTime;
	//2024-08-19  uchar -> int
	//signed char macroStatus = MACRO_INIT;
	int macroStatus = MACRO_INIT;

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

	bool Reset();

	//상태 정보 unsigned char x, int 형으로 변경
	//byte GetMacroStatus();
	int GetMacroStatus();
	bool MacroStart();
	bool MacroStop();
	//2024-08-19 매크로 정지 상태일 때 다시시작이 아닌 해당 지점부터 다시 재생
	bool MacroPlay();
	
//protected:
	bool MacroUpdate();
	bool MacroRandom();

public:
	bool SetMacroTime(int tickTime);
	unsigned int GetMacroTime();

	//키타입  byte -> WORD 형으로 변경
	//bool RegisterMacroKey(const byte key);
	//bool RegisterMacroKey(const byte key, const bool up = false);
	bool RegisterMacroKey(const WORD key, const bool up = false);
	//bool RegisterMacroKey(const DWORD time, const WPARAM keyType, const byte key,const DWORD scanCode);
	bool RegisterMacroKey(const DWORD time, const WPARAM keyType, const WORD key,const DWORD scanCode);
	bool RegisterCurrentAlgorithm();

	byte GetAlgorithmCount();
	bool SelectAlgorithm(int idx);
	

	vector<EXTENDED_INPUT> GetRegisterInputs();
	
}ActivateMacro;