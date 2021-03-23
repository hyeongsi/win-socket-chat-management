#include "Client.h"

using namespace std;

extern HWND g_hDlg;

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

void Client::SendLoginSignToServer(string id, string pw)
{
	Json::Value root;
	root["id"] = id;
	root["pw"] = pw;
	root["kind"] = Login;

	SendPacketToServer(root);
}

bool Client::SendMessageToServer(std::string msg)
{
	if (!msg.size())
		return false;

	Json::Value root;
	root["kind"] = Message;
	root["message"] = msg.c_str();
	// 채팅방 번호도 나중에 포함시키기

	return SendPacketToServer(root);
}

bool Client::SendPacketToServer(Json::Value root)
{
	char cBuffer[PACKET_SIZE];
	string str;

	Json::StyledWriter writer;
	str = writer.write(root);

	memcpy(cBuffer, str.c_str(), str.size());

	if (send(clientSocket, cBuffer, PACKET_SIZE, 0) == -1)
	{
		MessageBox(g_hDlg, "send error", "error", NULL);
		// 서버 재접속 코드 작성
		return false;
	}

	return true;
}

bool Client::SendFileDataToServer(FILE* fp, int fileSize)
{
	int sendBytes;	// 읽어온 파일 사이즈 저장할 변수
	char cBuffer[PACKET_SIZE];
	
	snprintf(cBuffer, sizeof(cBuffer), "%d", fileSize);

	while (1)
	{
		sendBytes = fread(cBuffer, sizeof(char), PACKET_SIZE, fp);
		if (send(clientSocket, cBuffer, sendBytes, 0) == -1)
		{
			MessageBox(g_hDlg, "send error", "error", NULL);
			// 서버 재접속 코드 작성
			return false;
		}

		if (feof(fp))
			break;
	}

	return true;
}

Json::Value Client::RecvPacketToServer()
{
	char cBuffer[PACKET_SIZE] = {};
	int recvResult;

	if ((recvResult = recv(clientSocket, cBuffer, PACKET_SIZE, 0)) != -1)
	{
		Json::Reader reader;
		Json::Value recvValue;
		reader.parse(cBuffer, recvValue);

		return recvValue;
	}

	return NULL;
}


