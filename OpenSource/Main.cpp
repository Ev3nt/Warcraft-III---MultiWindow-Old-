#include <Windows.h>
#include <Storm.h>

#include "CRedirector.h"

#pragma comment(lib, "wsock32.lib")

#define strcmpi _strcmpi

HMODULE hGame = LoadLibrary("Game.dll");

unsigned short g_tcp_port;

template<typename Return, typename Function, typename ... Arguments>
Return inline stdcall(Function function, Arguments ... arguments)
{
	return reinterpret_cast<Return(CALLBACK*)(Arguments...)>(function)(arguments...);
}

HWND WINAPI CreateWindowExA_Proxy(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
BOOL WINAPI SetWindowTextA_Proxy(HWND hWnd, LPCSTR lpString);
HCURSOR WINAPI LoadCursorA_Proxy(HINSTANCE hInstance, LPCSTR lpCursorName);
HANDLE WINAPI LoadImageA_Proxy(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);

BOOL WINAPI WSockBind_proxy(SOCKET s, const struct sockaddr FAR* name, int namelen);
BOOL WINAPI WSockSendTo_proxy(SOCKET s, const char FAR* buf, int len, int flags, const struct sockaddr FAR* to, int tolen);
HANDLE WINAPI CreateEventA_proxy(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);

//----------------------------------------------------------------------------------------------------------

BOOL WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	if (!hGame)
		return FALSE;

	FARPROC GameMain = GetProcAddress(hGame, "GameMain");

	if (!GameMain)
		return FALSE;

	CRedirector redirector;

	redirector.setModule(hGame);

	if (redirector.setApiModule("user32.dll"))
	{
		redirector.redirect("CreateWindowExA", CreateWindowExA_Proxy);
		redirector.redirect("SetWindowTextA", SetWindowTextA_Proxy);
		redirector.redirect("LoadCursorA", LoadCursorA_Proxy);
		redirector.redirect("LoadImageA", LoadImageA_Proxy);
	}

	if (redirector.setApiModule("wsock32.dll"))
	{
		redirector.redirect(2, WSockBind_proxy); 
		redirector.redirect(20, WSockSendTo_proxy);
	}

	if (redirector.setApiModule("kernel32.dll"))
		redirector.redirect("CreateEventA", CreateEventA_proxy);

	StormOpenArchive("", NULL, NULL, NULL);

	stdcall<BOOL>(GameMain, hGame);
	FreeLibrary(hGame);

	return FALSE;
}

//----------------------------------------------------------------------------------------------------------

HWND WINAPI CreateWindowExA_Proxy(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!strcmpi(lpWindowName, "warcraft iii"))
		return CreateWindowEx(dwExStyle, lpClassName, "Warcraft III - MultiWindow", dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	return CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

BOOL WINAPI SetWindowTextA_Proxy(HWND hWnd, LPCSTR lpString)
{
	if (!strcmpi(lpString, "warcraft iii"))
		return SetWindowText(hWnd, "Warcraft III - MultiWindow");

	return SetWindowText(hWnd, lpString);
}

HCURSOR WINAPI LoadCursorA_Proxy(HINSTANCE hInstance, LPCSTR lpCursorName)
{
	if (!strcmpi(lpCursorName, "blizzardcursor.cur"))
		return LoadCursor(hInstance, "MultiWindow.cur");

	return LoadCursor(hInstance, lpCursorName);
}

HANDLE WINAPI LoadImageA_Proxy(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad)
{
	if (!strcmpi(name, "war3x.ico"))
			return LoadImage(hInst, "MultiWindow.ico", type, cx, cy, LR_DEFAULTSIZE);

	return LoadImage(hInst, name, type, cx, cy, fuLoad);
}

BOOL WINAPI WSockBind_proxy(SOCKET s, const struct sockaddr FAR* name, int namelen)
{
	int optVal;
	int optLen = sizeof(int);

	if (SOCKET_ERROR != getsockopt(s, SOL_SOCKET, SO_TYPE, (LPSTR)&optVal, &optLen))
	{
		if (optVal == SOCK_DGRAM)
		{
			if (((struct sockaddr_in*)name)->sin_port == 0xE017)
			{
				BOOL optval = TRUE;
				setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (LPCSTR)&optval, sizeof(optval));

				return stdcall<int>(GetProcAddress(GetModuleHandle("wsock32.dll"), (LPCSTR)2), s, name, namelen);
			}
		}
		else if (optVal == SOCK_STREAM)
		{
			BOOL optval = FALSE;
			setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));

			unsigned short port = ntohs(((struct sockaddr_in*)name)->sin_port);

			while (true)
			{
				if (stdcall<int>(GetProcAddress(GetModuleHandle("wsock32.dll"), (LPCSTR)2), s, name, namelen) != SOCKET_ERROR)
				{
					g_tcp_port = port;

					return NULL;
				}

				((struct sockaddr_in*)name)->sin_port = htons(++port);
			}
		}
	}

	return stdcall<int>(GetProcAddress(GetModuleHandle("wsock32.dll"), (LPCSTR)2), s, name, namelen);
}

BOOL WINAPI WSockSendTo_proxy(SOCKET s, const char FAR* buf, int len, int flags, const struct sockaddr FAR* to, int tolen)
{
	struct war3_packet
	{
		unsigned char f7_;
		unsigned char cmd_;
		unsigned short size_;
	};

	struct war3_packet* data_ptr = (struct war3_packet*)buf;

	if (len >= sizeof(war3_packet) && data_ptr->f7_ == 0xF7)
	{
		if (data_ptr->cmd_ == 0x30)
		{
			*(unsigned short*)(&buf[data_ptr->size_ - 2]) = g_tcp_port;
			sockaddr_in addr = *(sockaddr_in*)to;
			addr.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");

			return stdcall<int>(GetProcAddress(GetModuleHandle("wsock32.dll"), (LPCSTR)20), s, (LPSTR)buf, len, flags, (const sockaddr*)&addr, sizeof(sockaddr_in));
		}
	}

	return stdcall<int>(GetProcAddress(GetModuleHandle("wsock32.dll"), (LPCSTR)20), s, buf, len, flags, to, tolen);
}

HANDLE WINAPI CreateEventA_proxy(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName)
{
	HANDLE retval = stdcall<HANDLE>(GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateEventA"), lpEventAttributes, bManualReset, bInitialState, lpName);

	if ((lpName) && (!strcmp(lpName, "Warcraft III Game Application")))
		SetLastError(NULL);

	return retval;
}