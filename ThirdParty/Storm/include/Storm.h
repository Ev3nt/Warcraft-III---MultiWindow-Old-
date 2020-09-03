#pragma once

#ifndef __STORM_H__
#define __STORM_H__

#include <windows.h>
#pragma comment(lib, "Storm.lib")

#ifndef STORM_ALTERNATE_NAMES
#define SFILE(Name) Storm##Name
#define SCOMP(Name) Storm##Name
#else
#define SFILE(Name) SFile##Name
#define SCOMP(Name) SComp##Name
#endif

#ifdef __cplusplus
extern "C" {
#endif

	BOOL  WINAPI SFILE(OpenArchive)(LPCSTR lpArchiveName, DWORD dwPriority, DWORD dwFlags, HANDLE *hMPQ);
	BOOL  WINAPI SFILE(CloseArchive)(HANDLE hMPQ);
	BOOL  WINAPI SFILE(GetArchiveName)(HANDLE hMPQ, LPCSTR lpBuffer, DWORD dwBufferLength);
	BOOL  WINAPI SFILE(OpenFile)(LPCSTR lpFileName, HANDLE *hFile);
	BOOL  WINAPI SFILE(OpenFileEx)(HANDLE hMPQ, LPCSTR lpFileName, DWORD dwSearchScope, HANDLE *hFile);
	BOOL  WINAPI SFILE(CloseFile)(HANDLE hFile);
	DWORD WINAPI SFILE(GetFileSize)(HANDLE hFile, LPDWORD lpFileSizeHigh);
	BOOL  WINAPI SFILE(GetFileArchive)(HANDLE hFile, HANDLE *hMPQ);
	BOOL  WINAPI SFILE(GetFileName)(HANDLE hFile, LPCSTR lpBuffer, DWORD dwBufferLength);
	DWORD WINAPI SFILE(SetFilePointer)(HANDLE hFile, long lDistanceToMove, PLONG lplDistanceToMoveHigh, DWORD dwMoveMethod);
	BOOL  WINAPI SFILE(ReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
	LCID  WINAPI SFILE(SetLocale)(LCID nNewLocale);
	BOOL  WINAPI SFILE(GetBasePath)(LPCSTR lpBuffer, DWORD dwBufferLength);
	BOOL  WINAPI SFILE(SetBasePath)(LPCSTR lpNewBasePath);

	BOOL  WINAPI SCOMP(Compress)  (char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength, int uCmp, int uCmpType, int nCmpLevel);
	BOOL  WINAPI SCOMP(Decompress)(char * pbOutBuffer, int * pdwOutLength, char * pbInBuffer, int dwInLength);

#ifdef __cplusplus
}
#endif

#endif