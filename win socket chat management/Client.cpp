#include "Client.h"

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\json_vc71_libmtd.lib")

using namespace std;

extern HWND g_hDlg;

enum MessageKind
{
	Message,
	File,
	Emoticon,
};

constexpr const int PACKET_SIZE = 1024;

Client* Client::instance = nullptr;
Client::Client(){ }

Client* Client::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new Client();
    }

    return instance;
}

void Client::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

bool Client::ConnectInit(string ip, int port)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) != 0)
		return false;

	return true;
}

void Client::CloseSocket()
{
	if (clientSocket != NULL)
		return;

	closesocket(clientSocket);
	WSACleanup();
}

void Client::SendMessageToServer(std::string msg)
{
	char cBuffer[PACKET_SIZE];
	string str;

	Json::Value root;
	root["id"] = "test";
	root["name"] = 123;
	root["kind"] = Message;
	root["message"] = msg.c_str();
	// 채팅방 번호도 나중에 포함시키기

	Json::StyledWriter writer;
	str = writer.write(root);

	memcpy(cBuffer, str.c_str(), str.size());

	if (send(clientSocket, cBuffer, PACKET_SIZE, 0) == -1)
	{
		MessageBox(g_hDlg, "send error", "error", NULL);
		// 서버 재접속 코드 작성
	}
}


