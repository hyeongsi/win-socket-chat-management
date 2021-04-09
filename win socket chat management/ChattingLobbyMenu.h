#pragma once
#include <WinSock2.h>
#include <mutex>
#include <string>
#include <vector>
#include "Client.h"

using namespace std;

typedef struct ChattingRoomHwnd
{
	HWND hwnd = NULL;
	int roomNumber;
	string roomName;
	bool turnOn = false;
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber) : hwnd(_hwnd), roomNumber(_roomNumber) {};
	ChattingRoomHwnd(HWND _hwnd, int _roomNumber, string _roomName)
		: hwnd(_hwnd), roomNumber(_roomNumber), roomName(_roomName) {};
}chattingRoomHwnd;

typedef struct DownLoadFileLine
{
	int line;
	string fileName;
	int roomNumber;
	DownLoadFileLine(int _line, string _fileName, int _roomNumber)
		: line(_line), fileName(_fileName), roomNumber(_roomNumber) {};
}downLoadFileLine;

class ChattingLobbyMenu
{
private:
	static ChattingLobbyMenu* instance;
	HWND hChatLobbyDlg;
	bool isFriendList = false;

	ChattingLobbyMenu();
public:
	HINSTANCE g_hInst;
	mutex chattingMutex;
	string myId;
	char inputFriendId[PACKET_SIZE] = "";
	char inputRoomName[PACKET_SIZE] = "";
	vector<chattingRoomHwnd> chattingDlgVector;
	vector<downLoadFileLine> downLoadFileLineVector;
	vector<string> friendVector;

	static ChattingLobbyMenu* GetInstance();
	static void ReleaseInstance();

	void SetInst(HINSTANCE g_hInst);

	static BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static void ChattingLobbyInit(HWND hDlg);
	void ChattingLobbyInitMethod(HWND hDlg);
	static void FriendListInit(HWND hDlg);
	void FriendListInitMethod(HWND hDlg);
	static void AddFriendButton(HWND hDlg);
	void AddFriendBtnMethod(HWND hDlg);
	static void ClickChattingRoom(HWND hDlg);
	void ClickChattingRoomMethod(HWND hDlg);
	static void AddChattingRoomButton(HWND hDlg);
	void AddChattingRoomBtnMethod(HWND hDlg);
	static void FindFriendButton(HWND hDlg);
	void FindFriendBtnMethod(HWND hDlg);
	static void FindChattingRoomButton(HWND hDlg);
	void FindChattingRoomBtnMethod(HWND hDlg);

	vector<string> SplitString(string input, char delimiter);

	static unsigned __stdcall RecvMessageThread(void* arg);
	static void RecvMessage(Json::Value recvJson);
	void RecvMessageMethod(Json::Value recvJson);
	static void AddFriendMessage(Json::Value recvJson);
	void AddFriendMessageMethod(Json::Value recvJson);
	static void AddChattingRoomMessage(Json::Value recvJson);
	void AddChattingRoomMessageMethod(Json::Value recvJson);
	static void AddChattingRoomUserMessage(Json::Value recvJson);
	void AddChattingRoomUserMessageMethod(Json::Value recvJson);
	static void GetFriendDataMessage(Json::Value recvJson);
	void GetFriendDataMessageMethod(Json::Value recvJson);
};

