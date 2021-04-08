#pragma once
#include <WinSock2.h>
#include "resource.h"

#include "ServerUI.h"
#include "Server.h"
#include "ChattingRoomManager.h"

using namespace std;

HWND g_hDlg;
HINSTANCE g_hInst;

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);