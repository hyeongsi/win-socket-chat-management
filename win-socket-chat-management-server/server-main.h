#pragma once
#include <WinSock2.h>
#include <process.h>
#include <string>
#include <list>
#include <time.h>
#include <cstdio>
#include <mutex>
#include "ChattingRoomManager.h"
#include "MembershipDB.h"
#include "resource.h"

#include "ServerUI.h"
#include "Server.h"

using namespace std;

enum SignUpMessage
{
	Success = 0,
	ExsistsSameId = 1, 
	ExsistsSameName = 2,
};

enum EmoticonKind
{
	Happy = 0,
	Sad = 1,
	Surprised = 2,
};

enum MessageKind
{
	SignUp = 0,
	Login = 1,
	ChattingRoomInit = 2,
	AddChattingRoom = 3,
	AddChattingRoomUser = 4,
	GetChattringRoomName = 5,
	GetFriendData = 6,
	Message = 7,
	GetFileRequest = 8,
	SetFileRequest = 9,
	Emoticon = 10,
	AddFriend = 11,
};

HWND g_hDlg;
HINSTANCE g_hInst;
HANDLE AcceptThreadHandle;
mutex clientSocketListMutex;

WSADATA wsaData;
SOCKET clientSocket;
SOCKADDR_IN clientAddress = {};
SOCKET serverSocket;
SOCKADDR_IN serverAddress = {};

bool isOpenServer = false;

time_t curTime = time(NULL);
struct tm localTime = *localtime(&curTime);

bool InitServer();
unsigned WINAPI RecvThread(void* arg);
unsigned WINAPI StartServer(void* arg);

void StopServer();

void SignUpMessageMethod(Json::Value recvValue, SOCKET* clientSocket);
void LoginMessageMethod(Json::Value recvValue, string* userId, string* userName, SOCKET* clientSocket);
void JsonMessageMethod(Json::Value recvValue, string* userId, string* userName);
void SetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket);
void AddFriendMessageMethod(Json::Value recvValue, string* userId, SOCKET* clientSocket);
void GetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket);
void ChattingRoomInitMethod(Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket);
void GetChattingRoomNameMethod(Json::Value recvValue, Json::Value sendValue, SOCKET* clientSocket);
void AddChattingRoomMethod(Json::Value recvValue, Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket);
void AddChattingRoomUserMethod(Json::Value recvValue, Json::Value sendValue, string* userId, SOCKET* clientSocket);
void GetFriendDataMethod(Json::Value sendValue, string* userId, SOCKET* clientSocket);
void EmoticonMessageMethod(Json::Value recvValue, Json::Value sendValue, string* userName, SOCKET* clientSocket);

void ExitClient(SOCKET* clientSocket);

void DebugLogUpdate(int kind, string message);
int CheckSignUpData(string id, string pw, string name);
string JsonToString(Json::Value value);
bool SendJsonData(Json::Value value, SOCKET socket);
bool SendFileDataToServer(FILE* fp, int fileSize, SOCKET socket);
string GetUserIdInUserList();
void DebugLogUpdate(int kind, string message);