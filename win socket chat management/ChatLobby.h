#pragma once

#include "Chat.h"
#include <sstream>
#include <vector>
#include <mutex>

typedef struct ChattingRoomHwnd
{
	HWND hwnd = NULL;
	int roomNumber;
	bool turnOn = false;
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

using namespace std;

BOOL CALLBACK InputIDDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InputRoomNameDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
unsigned WINAPI RecvMessageThread(void* arg);

vector<string> SplitString(string input, char delimiter);
void ChattingLobbyInit(HWND hDlg);
void FriendListInit(HWND hDlg);
void AddFriendBtnMethod(HWND hDlg);
void ClickChattingRoomMethod(HWND hDlg);
void AddChattingRoomBtnMethod(HWND hDlg);
void AddChattingRoomMethod(Json::Value recvJson);
void AddChattingRoomUserMethod(Json::Value recvJson);
void GetFriendDataMethod(Json::Value recvJson);
void RecvMessageMethod(Json::Value recvJson);
void AddFriendMethod(Json::Value recvJson);