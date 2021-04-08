#pragma once
#include <WinSock2.h>
#include <list>
#include <string>
#include <mutex>
#include "ServerEnumCollection.h"
#include "jsoncppkor\include\json\json.h"

constexpr const int PORT = 4567;
constexpr const int PACKET_SIZE = 1024;

using namespace std;

typedef struct UserDatas
{
	SOCKET socket;
	string id;
	string name;
	UserDatas(SOCKET _socket, string _id, string _name)
		: socket(_socket), id(_id), name(_name) {};
}UserData;

class Server
{
private:
	static Server* instance;
	bool isOpenServer = false;

	SOCKET clientSocket;
	SOCKET serverSocket;
	SOCKADDR_IN clientAddress = {};
	SOCKADDR_IN serverAddress = {};

	Server();
public:
	WSADATA wsaData;
	list<UserData> clientSocketList;
	mutex clientSocketListMutex;

	static Server* GetInstance();
	static void ReleaseInstance();

	bool InitServer();
	static unsigned WINAPI StartServer(void* arg);
	void StartServerMethod();
	static unsigned WINAPI RecvThread(void* arg);
	void RecvThreadMethod(SOCKET clientSocket);
	void StopServer();

	void CloseServerSocket();
private:
	int CheckSignUpData(string id, string pw, string name);
	string JsonToString(Json::Value value);	// json -> string
	bool SendJsonData(Json::Value value, SOCKET socket); // json send socket
	bool SendFileDataFromServer(FILE* fp, int fileSize, SOCKET socket); //file data send socket
	void ExitClient(SOCKET* clientSocket); // client socket ���� ó��

	void SignUpMessageMethod(Json::Value recvValue, SOCKET* clientSocket); // ȸ������ ó��
	void LoginMessageMethod(Json::Value recvValue, string* userId, string* userName, SOCKET* clientSocket); // �α��� ó��
	void ChattingRoomInitMethod(Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket); // ä�ù� �ʱ�ȭ
	void AddChattingRoomMethod(Json::Value recvValue, Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket); // ä�ù� �߰�
	void AddChattingRoomUserMethod(Json::Value recvValue, Json::Value sendValue, string* userId, SOCKET* clientSocket); // ä�ù濡 ���� �߰�
	void GetChattingRoomNameMethod(Json::Value recvValue, Json::Value sendValue, SOCKET* clientSocket); // ä�ù��̸� ����
	void GetFriendDataMethod(Json::Value sendValue, string* userId, SOCKET* clientSocket); // ģ�� ������ ����
	void JsonMessageMethod(Json::Value recvValue, string* userId, string* userName); // �޽��� ����
	void SetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket); // ���� ������ ����
	void AddFriendMessageMethod(Json::Value recvValue, string* userId, SOCKET* clientSocket); // ģ�� �߰�
	void GetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket); // ���� ������ �۽�
	void EmoticonMessageMethod(Json::Value recvValue, Json::Value sendValue, string* userName, SOCKET* clientSocket); // �̸�Ƽ�� ó��
};