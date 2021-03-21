#pragma once
#include <WinSock2.h>
#include <string>

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\json_vc71_libmtd.lib")

enum MessageKind
{
	SignUp,
	Login,
	Message,
	File,
	Emoticon,
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
	static Client* GetInstance();
	static void ReleaseInstance();

	bool ConnectInit(std::string ip, int port);
	void CloseSocket();

	void SendLoginSignToServer(std::string id, std::string pw);
	bool SendMessageToServer(std::string msg);

	bool SendPacketToServer(Json::Value root);
	Json::Value RecvPacketToServer();
};