#pragma once
#include <WinSock2.h>
#include <process.h>
#include <string>
#include <list>
#include <time.h>
#include "MembershipDB.h"
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

enum MessageKind
{
	SignUp,
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

int CheckSignUpData(string id, string pw, string name)
{
	switch (MembershipDB::GetInstance()->ExistValue(ID, id))
	{
	case ID:
		return ExsistsSameId;
	case -2:
		return -2;
	}

	switch (MembershipDB::GetInstance()->ExistValue(NAME, name))
	{
	case NAME:
		return ExsistsSameName;
	case -2:
		return -2;
	}

	return Success;
}

string JsonToString(Json::Value value)
{
	string str;
	Json::StyledWriter writer;
	str = writer.write(value);

	return str;
}

bool SendJsonData(Json::Value value)
{
	string jsonString;
	char cBuffer[PACKET_SIZE] = {};
	jsonString = JsonToString(value);
	memcpy(cBuffer, jsonString.c_str(), jsonString.size());

	if (send(clientSocket, cBuffer, PACKET_SIZE, 0) == -1)
		return false;
	else
		return true;
}

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET clientSocket = *(SOCKET*)arg;
	char cBuffer[PACKET_SIZE] = {};
	string userId, userName;

	while ((recv(clientSocket, cBuffer, PACKET_SIZE, 0)) != -1)
	{
		if (!isOpenServer)
		{
			closesocket(clientSocket);
			return 0;
		}

		Json::Reader reader;
		Json::Value recvValue, sendValue;
		reader.parse(cBuffer, recvValue);

		switch (recvValue["kind"].asInt())
		{
		case SignUp:
			DebugLogUpdate(logBox, recvValue["value"].asString() + ", " + recvValue["id"].asString() + ", "+
				recvValue["pw"].asString() + ", "+ recvValue["name"].asString() + " 회원가입 요청");
			
			sendValue["value"] = CheckSignUpData(recvValue["id"].asString(),
				recvValue["pw"].asString(), recvValue["name"].asString());

			if (Success == sendValue["value"].asInt())
			{
				MembershipDB::GetInstance()->WriteMembershipData(
					recvValue["id"].asString(), recvValue["pw"].asString(), recvValue["name"].asString());
				DebugLogUpdate(logBox, "회원가입 성공");
				// db에 데이터 저장;
				sendValue["result"] = true;
			}
			else
			{
				DebugLogUpdate(logBox, "회원가입 실패");
				sendValue["result"] = false;
			}
				
			if (!SendJsonData(sendValue))
				return 0;

			break;
		case Login:
			sendValue["result"] = MembershipDB::GetInstance()->
				LoginCheck(recvValue["id"].asString(),
					recvValue["pw"].asString());

			if (!SendJsonData(sendValue))
				return 0;

			if (LoginSuccess == sendValue["result"].asInt())
			{
				userId = recvValue["id"].asString();
				userName = MembershipDB::GetInstance()->FindName(recvValue["id"].asString());

				clientSocketList.emplace_back(UserData(
					clientSocket, 
					userId,
					userName));
				
				DebugLogUpdate(userBox, "id : " + userId + " name : " + userName);
				DebugLogUpdate(logBox, userId + ", " + userName + " 유저 접속");
			}

			break;
		case Message:
			DebugLogUpdate(logBox, userId + " / " + userName +
				" / message : " + recvValue["message"].asString());

			sendValue["kind"] = Message;
			sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
			sendValue["message"] = recvValue["message"].asString();
			if (!SendJsonData(sendValue))
				return 0;
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