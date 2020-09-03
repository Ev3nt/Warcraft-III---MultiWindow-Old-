#pragma once

#include <Windows.h>

// CRedirector error list
#define CREDIRECTOR_WRONG_API_MODULE	8
#define CREDIRECTOR_INVALID_THUNK		7
#define CREDIRECTOR_INVALID_FUNCTION	6
#define CREDIRECTOR_INVALID_FIRSTTHUNK	5
#define CREDIRECTOR_WRONG_NT_SIGNATURE	4
#define CREDIRECTOR_WRONG_DOS_SIGNATURE	3
#define CREDIRECTOR_INVALID_API_MODULE	2
#define CREDIRECTOR_INVALID_MODULE	1
#define CREDIRECTOR_SUCCESSFUL	0

// CRedirector
class CRedirector
{
private:
	HMODULE m_hModule = NULL;
	HMODULE m_hApiModule = NULL;
	PIMAGE_THUNK_DATA m_pThunk = NULL;
public:
	CRedirector() {};
	bool setModule(HMODULE hModule);
	bool setModule(LPCSTR lpModuleName);

	bool setApiModule(HMODULE hApiModule);
	bool setApiModule(LPCSTR lpApiModuleName);

	bool redirect(LPCSTR lpProcName, LPVOID lpNewProcAddress);
	bool redirect(int nProcOrdinal, LPVOID lpNewProcAddress);
};