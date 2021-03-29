#pragma once

#include "Chat.h"
#include <vector>

typedef struct ChattingRoomHwnd
{
	HWND hwnd;
	int roomNumber;
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber) : hwnd(_hwnd), roomNumber(_roomNumber) {};
}chattingRoomHwnd;

typedef struct DownLoadFileLine
{
	HWND hwnd;
	int line;
	std::string fileName;
	DownLoadFileLine(HWND _hwnd, int _line, std::string _fileName) : hwnd(_hwnd), line(_line), fileName(_fileName) {};
}downLoadFileLine;

extern HINSTANCE g_hInst;

BOOL CALLBACK InputIDDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
unsigned WINAPI RecvMessageThread(void* arg);