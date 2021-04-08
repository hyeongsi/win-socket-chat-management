#pragma once

#include "Chat.h"
#include <sstream>
#include <vector>
#include <mutex>

typedef struct ChattingRoomHwnd
{
	HWND hwnd = NULL;
	int roomNumber;
	std::string roomName;
	bool turnOn = false;
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber) : hwnd(_hwnd), roomNumber(_roomNumber) {};
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber, std::string _roomName)
		: hwnd(_hwnd), roomNumber(_roomNumber), roomName(_roomName) {};
}chattingRoomHwnd;

typedef struct DownLoadFileLine
{
	int line;
	std::string fileName;
	int roomNumber;
	DownLoadFileLine(int _line, std::string _fileName, int _roomNumber) 
		: line(_line), fileName(_fileName), roomNumber(_roomNumber) {};
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
void FriendBtnMethod(HWND hDlg);
void ChatBtnMethod(HWND hDlg);

void AddChattingRoomMethod(Json::Value recvJson);
void AddChattingRoomUserMethod(Json::Value recvJson);
void GetFriendDataMethod(Json::Value recvJson);
void RecvMessageMethod(Json::Value recvJson);
void AddFriendMethod(Json::Value recvJson);