#pragma once
#include <WinSock2.h>
#include <string>

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\lib_json.lib")

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

constexpr const int PACKET_SIZE = 1024;

class Client
{
private:
	static Client* instance;
	SOCKET clientSocket = NULL;

	WSADATA wsaData;
	SOCKADDR_IN serverAddress = {};

	Client();
public:
	HWND g_hDlg;

	static Client* GetInstance();
	static void ReleaseInstance();

	bool ConnectInit(std::string ip, int port);
	void CloseSocket();

	void SendLoginSignToServer(std::string id, std::string pw);
	bool SendMessageToServer(std::string msg, const int roomNumber);

	bool SendPacketToServer(Json::Value root);
	bool SendFileDataToServer(FILE* fp, int fileSize);
	bool RecvFileData(Json::Value value);
	Json::Value RecvPacketToServer();

	bool isExistFile(std::string filePath, int index);
};