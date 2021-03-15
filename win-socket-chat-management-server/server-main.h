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

	DebugLog("서버가 시작되었습니다.");
	isOpenServer = true;

	int clientAddressSize = sizeof(clientAddress);
	while (isOpenServer)
	{
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		// 로그인 정보 중 id값 가져와서 리스트에 해당 id값 넣어야함 지금은 임시로 123으로 설정
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
	DebugLog("서버가 종료되었습니다.");
}

void MoveScrollbarToEnd(HWND hwnd)
{
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void DebugLog(string message)
{
	string logMessage = to_string(1900 + localTime.tm_year) + "년" + to_string(localTime.tm_mon) + "월"
		+ to_string(localTime.tm_mday) + "일" + to_string(localTime.tm_hour) + "시"
		+ to_string(localTime.tm_min) + "분" + to_string(localTime.tm_sec) + "초" + " / "+ message;

	SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST2), LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
	MoveScrollbarToEnd(GetDlgItem(g_hDlg, IDC_LOG_LIST2));
}