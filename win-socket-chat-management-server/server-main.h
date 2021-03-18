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
	string id;
	UserDatas(SOCKET _socket, string _id) : socket(_socket), id(_id) {};
}UserData;

enum MessageKind
{
	Login,
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
	int recvResult;

	while ((recvResult = recv(clientSocket, cBuffer, PACKET_SIZE, 0)) != -1)
	{
		if (!isOpenServer)
		{
			closesocket(clientSocket);
			return 0;
		}

		Json::Reader reader;
		Json::Value root;
		reader.parse(cBuffer, root);

		switch (root["kind"].asInt())
		{
		case Login:
			clientSocketList.emplace_back(UserData(clientSocket, root["id"].asString()));
			DebugLogUpdate(logBox, root["id"].asString() + ", " + root["name"].asString() + ", 유저 접속");
			DebugLogUpdate(userBox, "id : " + root["id"].asString() + " name : " + root["name"].asString());
			break;
		case Message:
			DebugLogUpdate(logBox, "id : " + root["id"].asString() + " name : " + root["name"].asString()
				+ " kind : " + root["kind"].asString() + " message : " + root["message"].asString());

			// send 처리 해야 함
			break;
		case File:
			break;
		case Emoticon:
			break;
		default:
			return 0;
		}
	}

	int count = 0;
	for (auto iterator = clientSocketList.begin(); iterator != clientSocketList.end();)
	{
		if ((*iterator).socket == clientSocket)
		{
			DebugLogUpdate(logBox, (*iterator).id + "유저 로그아웃");
			clientSocketList.erase(iterator);
			SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_DELETESTRING, count, 0);
			break;
		}
		else
		{
			count++;
			iterator++;
		}
	}

	closesocket(clientSocket);

	return 0;
}

unsigned WINAPI StartServer(void * arg)
{
	if (isOpenServer)
		return 0;

	if (!InitServer())
		return 0;

	DebugLogUpdate(logBox,"서버가 시작되었습니다.");
	isOpenServer = true;

	int clientAddressSize = sizeof(clientAddress);
	while (isOpenServer)
	{
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		if (SOCKET_ERROR != clientSocket)
		{
			// 로그인 정보 중 id값 가져와서 리스트에 해당 id값 넣어야함 지금은 임시로 123으로 설정
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
	DebugLogUpdate(logBox, "서버가 종료되었습니다.");
}

void MoveScrollbarToEnd(HWND hwnd)
{
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void AdjustListboxHScroll(HWND hwnd)
{
	int nTextLen = 0, nWidth = 0;
	int nCount = 0, idx = 0;
	HDC hDc = NULL;
	HFONT hFont = NULL;
	SIZE sz = { 0 };
	char pszText[MAX_PATH] = { 0, };

	nCount = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
	hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	hDc = GetDC(hwnd);
	SelectObject(hDc, (HGDIOBJ)hFont);

	for (idx = 0; idx < nCount; idx++)
	{
		nTextLen = SendMessage(hwnd, LB_GETTEXTLEN, idx, 0);
		memset(pszText, 0, MAX_PATH);
		SendMessage(hwnd, LB_GETTEXT, idx, (LPARAM)pszText);
		GetTextExtentPoint32A(hDc, pszText, nTextLen, &sz);
		nWidth = max(sz.cx, nWidth);
	}

	ReleaseDC(hwnd, hDc);
	SendMessage(hwnd, LB_SETHORIZONTALEXTENT, nWidth + 20, 0);
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

		logMessage = to_string(1900 + localTime.tm_year) + "년" + to_string(localTime.tm_mon) + "월"
			+ to_string(localTime.tm_mday) + "일" + to_string(localTime.tm_hour) + "시"
			+ to_string(localTime.tm_min) + "분" + to_string(localTime.tm_sec) + "초" + " / " + message;
		break;
	default:
		return;
	}

	SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
	MoveScrollbarToEnd(listBox);
	AdjustListboxHScroll(listBox);
}