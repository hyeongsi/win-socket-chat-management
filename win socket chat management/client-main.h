#pragma once

#include "Login.h"
#include "SignUp.h"
#include "Chat.h"
#include "resource.h"

#pragma comment(lib, "ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

constexpr const int ipEditboxCharSize = 30;

HWND g_hDlg;
HINSTANCE g_hInst;
bool isLogin;