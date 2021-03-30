#pragma once

#include "Login.h"
#include "SignUp.h"
#include "ChatLobby.h"
#include "resource.h"

#pragma comment(lib, "ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
void ClientLoginBtnMethod(HWND hDlg, WPARAM wParam);

HWND g_hDlg;
HINSTANCE g_hInst;
bool isLogin;