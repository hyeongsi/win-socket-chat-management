#pragma once
#include <WinSock2.h>
#include <process.h>
#include <string>
#include <list>
#include <time.h>
#include "resource.h"

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\json_vc71_libmtd.lib")

using namespace std;

constexpr const int PORT = 4567;

enum listboxKind
{
	userBox,
	logBox,
};

typedef struct UserDatas
{
	SOCKET socket;
	int id;
	UserDatas(SOCKET _socket, int _id) : socket(_socket), id(_id) {};
}UserData;

enum MessageKind
{
	Message,
	File,
	Emoticon,
};

constexpr const int PACKET_SIZE = 1024;

HWND g_hDlg;
HANDLE AcceptThreadHandle;

WSADATA wsaData;
SOCKET clientSocket;
SOCKADDR_IN clientAddress = {};
SOCKET serverSocket;
SOCKADDR_IN serverAddress = {};
list<UserData> clientSocketList;

bool isOpenServer = false;

time_t curTime = time(NULL);
struct tm localTime = *localtime(&curTime);

void DebugLogUpdate(int kind, string message);

bool InitServer()
{
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 == bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)))
	{
		MessageBox(g_hDlg, "bind Error", "error", NULL);
		return false;
	}

	if (-1 == listen(serverSocket, SOMAXCONN))
	{
		MessageBox(g_hDlg, "listen Error", "error", NULL);
		return false;
	}

	return true;
}

string GetMyIP()
{
	char* ip = nullptr;
	char name[255];
	PHOSTENT host;

	if (gethostname(name, sizeof(name)) == 0)
	{
		if ((host = gethostbyname(name)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
		}
	}

	if (nullptr == ip)
		return "";
		
	return string(ip);
}

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET clientSocket = *(SOCKET*)arg;
	char cBuffer[PACKET_SIZE] = {};

	while (recv(clientSocket, cBuffer, PACKET_SIZE, 0) != -1)
	{
		// ���Ƿ� ��¥ id ��� ���̵��� ����ϴ� ��
		// json ���ڿ��� �ް�, �����ϰ�, ó���� �Ŀ� ����ϵ��� �����ؾ� ��
		// ������ �׽�Ʈ������ cBuffer �׳� �� ��� �ϴ� ��
		Json::Reader reader;
		Json::Value root;
		reader.parse(cBuffer, root);

		DebugLogUpdate(logBox, "id : " + root["id"].asString() + " name : " + root["name"].asString()
		+ " kind : " + root["kind"].asString() + " message : " + root["message"].asString());
		//DebugLogUpdate(logBox, "���̵�, �̸�," + string(cBuffer));
	}

	// userlistbox ���� �ش� ���� ���� ó��
	// clientSocketList���� ���� �� Ŭ���̾�Ʈ ���� ó��
	closesocket(clientSocket);

	return 0;
}

unsigned WINAPI StartServer(void * arg)
{
	if (isOpenServer)
		return 0;

	if (!InitServer())
		return 0;

	DebugLogUpdate(logBox,"������ ���۵Ǿ����ϴ�.");
	isOpenServer = true;

	int clientAddressSize = sizeof(clientAddress);
	while (isOpenServer)
	{
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		if (SOCKET_ERROR != clientSocket)
		{
			// �α��� ���� �� id�� �����ͼ� ����Ʈ�� �ش� id�� �־���� ������ �ӽ÷� 123���� ����
			clientSocketList.emplace_back(UserData(clientSocket, 123));
			DebugLogUpdate(logBox, "test, 123 , ���� ����");
			DebugLogUpdate(userBox, "id : test , name : 123");
			_beginthreadex(NULL, 0, RecvThread, &clientSocket, 0, NULL);
		}
	}
	
	return 0;
}

void StopServer()
{
	if (!isOpenServer)
		return;
	
	int count = SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCOUNT, 0, 0);
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_DELETESTRING, 0, 0);
		}
	}

	closesocket(serverSocket);
	AcceptThreadHandle = nullptr;
	clientSocketList.clear();
	isOpenServer = false;
	DebugLogUpdate(logBox, "������ ����Ǿ����ϴ�.");
}

void MoveScrollbarToEnd(HWND hwnd)
{
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void DebugLogUpdate(int kind, string message)
{
	HWND listBox;
	string logMessage;

	switch (kind)
	{
	case userBox :
		listBox = GetDlgItem(g_hDlg, IDC_USERS_LIST);
		logMessage = message;
		break;
	case logBox:
		listBox = GetDlgItem(g_hDlg, IDC_LOG_LIST);

		logMessage = to_string(1900 + localTime.tm_year) + "��" + to_string(localTime.tm_mon) + "��"
			+ to_string(localTime.tm_mday) + "��" + to_string(localTime.tm_hour) + "��"
			+ to_string(localTime.tm_min) + "��" + to_string(localTime.tm_sec) + "��" + " / " + message;
		break;
	default:
		return;
	}

	SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
	MoveScrollbarToEnd(listBox);
}