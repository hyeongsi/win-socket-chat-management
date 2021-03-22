#pragma once

#include "Chat.h"
#include <vector>

typedef struct ChattingRoomHwnd
{
	HWND hwnd;
	int roomNumber;
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber) : hwnd(_hwnd), roomNumber(_roomNumber) {};
}chattingRoomHwnd;

extern HINSTANCE g_hInst;

BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
unsigned WINAPI RecvMessageThread(void* arg);