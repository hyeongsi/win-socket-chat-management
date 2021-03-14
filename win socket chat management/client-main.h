#pragma once

#include <WinSock2.h>
#include "Login.h"
#include "SignUp.h"
#include "Chat.h"
#include "Client.h"

#pragma comment(lib, "ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

constexpr const int ipEditboxCharSize = 30;

HINSTANCE g_hInst;
bool isLogin;