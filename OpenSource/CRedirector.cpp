#include "CRedirector.h"

bool CRedirector::setModule(HMODULE hModule)
{
	if (!hModule)
	{
		SetLastError(CREDIRECTOR_INVALID_MODULE);
		return false;
	}

	m_hModule = hModule;

	SetLastError(CREDIRECTOR_SUCCESSFUL);
	return true;
}

bool CRedirector::setModule(LPCSTR lpModuleName)
{
	return setModule(GetModuleHandle(lpModuleName));
}

bool CRedirector::setApiModule(HMODULE hApiModule)
{
	if (!hApiModule)
	{
		SetLastError(CREDIRECTOR_INVALID_API_MODULE);
		return false;
	}

	m_hApiModule = hApiModule;

	if (!m_hModule)
	{
		SetLastError(CREDIRECTOR_INVALID_MODULE);
		return false;
	}

	if (PIMAGE_DOS_HEADER(m_hModule)->e_magic != IMAGE_DOS_SIGNATURE)
	{
		SetLastError(CREDIRECTOR_WRONG_DOS_SIGNATURE);
		return false;
	}

	PIMAGE_NT_HEADERS pNTHeader = PIMAGE_NT_HEADERS(PBYTE(m_hModule) + PIMAGE_DOS_HEADER(m_hModule)->e_lfanew);
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		SetLastError(CREDIRECTOR_WRONG_NT_SIGNATURE);
		return false;
	}

	DWORD dwImportRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)(m_hModule) + (ULONG_PTR)(dwImportRVA));

	while (pImportDesc->FirstThunk)
	{
		if (GetModuleHandle((PSTR)((ULONG_PTR)(m_hModule)+(ULONG_PTR)(pImportDesc->Name))) == hApiModule)
			break;

		pImportDesc++;
	}

	if (!pImportDesc->FirstThunk)
	{
		SetLastError(CREDIRECTOR_INVALID_FIRSTTHUNK);
		return false;
	}

	m_pThunk = (PIMAGE_THUNK_DATA)((PBYTE)m_hModule + (DWORD)pImportDesc->FirstThunk);

	SetLastError(CREDIRECTOR_SUCCESSFUL);
	return true;
}

bool CRedirector::setApiModule(LPCSTR lpApiModuleName)
{
	return setApiModule(GetModuleHandle(lpApiModuleName));
}

bool CRedirector::redirect(LPCSTR lpProcName, LPVOID lpNewProcAddress)
{
	ULONG_PTR uNewProcAddress = (ULONG_PTR)lpNewProcAddress;
	ULONG_PTR uRoutineAddress = (ULONG_PTR)GetProcAddress(m_hApiModule, lpProcName);
	PIMAGE_THUNK_DATA pThunk = m_pThunk;

	if (!uRoutineAddress)
	{
		SetLastError(CREDIRECTOR_INVALID_FUNCTION);
		return false;
	}

	if (!pThunk)
	{
		SetLastError(CREDIRECTOR_INVALID_THUNK);
		return false;
	}

	while (pThunk->u1.Function)
	{
		ULONG_PTR* uAddress = (ULONG_PTR*)&pThunk->u1.Function;
		if (*uAddress == uRoutineAddress)
		{
			DWORD dwOldProtect = NULL;

			VirtualProtect((LPVOID)uAddress, 4, PAGE_WRITECOPY, &dwOldProtect);
			*uAddress = uNewProcAddress;
			VirtualProtect((LPVOID)uAddress, 4, dwOldProtect, NULL);

			SetLastError(CREDIRECTOR_SUCCESSFUL);
			return true;
		}

		pThunk++;
	}

	SetLastError(CREDIRECTOR_WRONG_API_MODULE);
	return false;
}

bool CRedirector::redirect(int nProcOrdinal, LPVOID lpNewProcAddress)
{
	return redirect((LPCSTR)nProcOrdinal, lpNewProcAddress);
}