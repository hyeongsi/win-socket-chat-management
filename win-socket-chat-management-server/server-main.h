#pragma once
#include <WinSock2.h>
#include <process.h>
#include <string>
#include <list>
#include <time.h>
#include "resource.h"

using namespace std;

constexpr const int PORT = 4567;

typedef struct UserDatas
{
	SOCKET socket;
	int id;
	UserDatas(SOCKET _socket, int _id) : socket(_socket), id(_id) {};
}UserData;

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

void DebugLog(string message);

void InitServer(HWND hDlg)
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		MessageBox(hDlg, "WSAStartup Error", "error", NULL);
		return;
	}

	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 == bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)))
	{
		MessageBox(hDlg, "Bind Error", "error", NULL);
		return;
	}

	listen(serverSocket, SOMAXCONN);
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

unsigned WINAPI StartServer(void * arg)
{
	if (isOpenServer)
		return 0;

	DebugLog("������ ���۵Ǿ����ϴ�.");
	isOpenServer = true;

	int clientAddressSize = sizeof(clientAddress);
	while (isOpenServer)
	{
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		// �α��� ���� �� id�� �����ͼ� ����Ʈ�� �ش� id�� �־���� ������ �ӽ÷� 123���� ����
		clientSocketList.emplace_back(UserData(clientSocket,123));
	}
	
	return 0;
}

void StopServer()
{
	if (!isOpenServer)
		return;

	AcceptThreadHandle = nullptr;
	clientSocketList.clear();
	isOpenServer = false;
	DebugLog("������ ����Ǿ����ϴ�.");
}

void MoveScrollbarToEnd(HWND hwnd)
{
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void DebugLog(string message)
{
	string logMessage = to_string(1900 + localTime.tm_year) + "��" + to_string(localTime.tm_mon) + "��"
		+ to_string(localTime.tm_mday) + "��" + to_string(localTime.tm_hour) + "��"
		+ to_string(localTime.tm_min) + "��" + to_string(localTime.tm_sec) + "��" + " / "+ message;

	SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST2), LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
	MoveScrollbarToEnd(GetDlgItem(g_hDlg, IDC_LOG_LIST2));
}