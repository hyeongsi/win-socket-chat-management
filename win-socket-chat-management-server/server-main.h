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

using namespace std;

constexpr const int PORT = 4567;

const char* SAVE_LOG_PATH = "saveLog\\serverLogData.csv";

enum listboxKind
{
	userBox,
	logBox,
};

typedef struct UserDatas
{
	SOCKET socket;
	string id;
	string name;
	UserDatas(SOCKET _socket, string _id, string _name) 
		: socket(_socket), id(_id), name(_name) {};
}UserData;

enum SignUpMessage
{
	Success,
	ExsistsSameId,
	ExsistsSameName,
};

enum EmoticonKind
{
	Happy,
	Sad,
	Surprised,
};

enum MessageKind
{
	SignUp,
	Login,
	ChattingRoomInit,
	AddChattingRoom,
	AddChattingRoomUser,
	GetChattringRoomName,
	GetFriendData,
	Message,
	GetFileRequest,
	SetFileRequest,
	Emoticon,
	AddFriend,
};

constexpr const int PACKET_SIZE = 1024;

HWND g_hDlg;
HINSTANCE g_hInst;
HANDLE AcceptThreadHandle;
mutex clientSocketListMutex, userIdDataVectorMutex;

WSADATA wsaData;
SOCKET clientSocket;
SOCKADDR_IN clientAddress = {};
SOCKET serverSocket;
SOCKADDR_IN serverAddress = {};
list<UserData> clientSocketList;
vector<string> getUserIdData;

bool isOpenServer = false;

time_t curTime = time(NULL);
struct tm localTime = *localtime(&curTime);

bool InitServer();
unsigned WINAPI RecvThread(void* arg);
unsigned WINAPI StartServer(void* arg);

void StopServer();
void InitDialogMethod(HWND hDlg);
void CheckConnectUserBtnMethod();
void CheckUserIdListBtnMethod();
void BanBtnMethod();
void UnBanBtnMethod();
void SaveServerLogBtnMethod();

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

string GetMyIP();
void DebugLogUpdate(int kind, string message);
int CheckSignUpData(string id, string pw, string name);
string JsonToString(Json::Value value);
bool SendJsonData(Json::Value value, SOCKET socket);
bool SendFileDataToServer(FILE* fp, int fileSize, SOCKET socket);
string GetUserIdInUserList();
void MoveScrollbarToEnd(HWND hwnd);
void AdjustListboxHScroll(HWND hwnd);
void DebugLogUpdate(int kind, string message);