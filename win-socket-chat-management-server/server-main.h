﻿#pragma once
#include <WinSock2.h>
#include <process.h>
#include <string>
#include <list>
#include <time.h>
#include <cstdio>
#include "MembershipDB.h"
#include "resource.h"

using namespace std;

constexpr const int PORT = 4567;

const char* SAVE_LOG_PATH = "saveLog\\serverLogData.csv";

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
	GetFileRequest,
	SetFileRequest,
	Emoticon,
	AddFriend,
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

string GetMyIP();
void DebugLogUpdate(int kind, string message);
int CheckSignUpData(string id, string pw, string name);
string JsonToString(Json::Value value);
bool SendJsonData(Json::Value value, SOCKET socket);
bool SendFileDataToServer(FILE* fp, int fileSize, SOCKET socket);
string GetUserIdInUserList();

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

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET clientSocket = *(SOCKET*)arg;
	char cBuffer[PACKET_SIZE] = {};
	string userId, userName;
	int totalRecvFileCount, currentRecvFileCount = 0;
	int fileSize;
	int readByteSize;
	int findReturnValue;

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
			DebugLogUpdate(logBox, recvValue["id"].asString() + ", "+ 
				recvValue["pw"].asString() + ", "+ recvValue["name"].asString() + " 회원가입 요청");
			
			sendValue["value"] = CheckSignUpData(recvValue["id"].asString(),
				recvValue["pw"].asString(), recvValue["name"].asString());

			if (Success == sendValue["value"].asInt())
			{
				vector<string> writeData;
				writeData.emplace_back(recvValue["id"].asString());
				writeData.emplace_back(recvValue["pw"].asString());
				writeData.emplace_back(recvValue["name"].asString());

				MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, writeData);
				writeData.clear();
				writeData.emplace_back(recvValue["id"].asString());
				MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->FRIEND_LIST_PATH, writeData);
				DebugLogUpdate(logBox, "회원가입 성공");
				// db에 데이터 저장;
				SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), BM_CLICK, 0, 0);
				sendValue["result"] = true;
			}
			else
			{
				DebugLogUpdate(logBox, "회원가입 실패");
				sendValue["result"] = false;
			}
				
			if (!SendJsonData(sendValue, clientSocket))
				return 0;

			break;
		case Login:
			sendValue["result"] = MembershipDB::GetInstance()->
				LoginCheck(recvValue["id"].asString(),
					recvValue["pw"].asString(), &sendValue);

			if (!SendJsonData(sendValue, clientSocket))
				return 0;

			if (LoginSuccess == sendValue["result"].asInt())
			{
				userId = recvValue["id"].asString();
				userName = MembershipDB::GetInstance()->FindName(recvValue["id"].asString());

				clientSocketList.emplace_back(UserData(
					clientSocket, 
					userId,
					userName));
				
				SendMessage(GetDlgItem(g_hDlg, ID_CONNECT_USER_CHECK_BTN), BM_CLICK, 0, 0);
				DebugLogUpdate(logBox, userId + ", " + userName + " 유저 접속");
			}

			sendValue["name"] = MembershipDB::GetInstance()->FindName(recvValue["id"].asString());
			if (!SendJsonData(sendValue, clientSocket))
				return 0;

			break;
		case Message:
			DebugLogUpdate(logBox, userId + " / " + userName +
				" / message : " + recvValue["message"].asString());

			sendValue["kind"] = Message;	
			sendValue["name"] = userName;
			sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
			sendValue["message"] = recvValue["message"].asString();

			for (auto iterator = clientSocketList.begin(); iterator != clientSocketList.end(); iterator++)
			{
				SendJsonData(sendValue, (*iterator).socket);
			}

			break;
		case SetFileRequest:
			currentRecvFileCount = 0;
			fileSize = recvValue["fileSize"].asInt();
			totalRecvFileCount = fileSize / PACKET_SIZE + 1;
			
			FILE* sfp;
			fopen_s(&sfp, ("downloadFiles\\" + recvValue["fileName"].asString()).c_str(), "wb");
			if (sfp != NULL)
			{
				while (currentRecvFileCount != totalRecvFileCount)
				{
					readByteSize = recv(clientSocket, cBuffer, PACKET_SIZE, 0);
					currentRecvFileCount++;
					fwrite(cBuffer, sizeof(char), readByteSize, sfp);
				}

				fclose(sfp);
			}

			DebugLogUpdate(logBox, userName + " " + 
				recvValue["fileName"].asString() + "수신완료");

			sendValue["kind"] = GetFileRequest;
			sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
			sendValue["message"] = userName + "님이 " +
				recvValue["fileName"].asString() + "파일을 보냈습니다.";
			sendValue["fileName"] = recvValue["fileName"].asString();
			sendValue["roomNumber"] = recvValue["roomNumber"].asInt();

			for (auto iterator = clientSocketList.begin(); iterator != clientSocketList.end(); iterator++)
			{
				SendJsonData(sendValue, (*iterator).socket);
			}
			break;
		case AddFriend:
			sendValue["kind"] = AddFriend;
			if (recvValue["friendId"].asString() == userId)
			{
				sendValue["result"] = false;
				sendValue["message"] = "자기 자신은 친구로 등록할 수 없습니다.";
				SendJsonData(sendValue, clientSocket);
				break;
			}

			findReturnValue = MembershipDB::GetInstance()->ExistValue(
				MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, ID, recvValue["friendId"].asString());

			if (findReturnValue < 0)
			{
				sendValue["result"] = false;
				sendValue["message"] = "해당 id는 없는 id 입니다.";
				SendJsonData(sendValue, clientSocket);
				break;
			}

			sendValue["result"] = true;
			SendJsonData(sendValue, clientSocket);
			break;
		case GetFileRequest:
			long fileSize;	// 파일 전체 사이즈
			FILE* fp;
			DebugLogUpdate(logBox, userName + "파일전송요청 수신");
			fopen_s(&fp, ("downloadFiles\\" + recvValue["fileName"].asString()).c_str(), "rb");	// 파일 열고
			if (fp != NULL)
			{	// 파일을 보냄
				DebugLogUpdate(logBox, userName + recvValue["fileName"]. asString() + " 파일전송시작");
				fseek(fp, 0, SEEK_END);
				fileSize = ftell(fp);
				fseek(fp, 0, SEEK_SET);

				sendValue["kind"] = SetFileRequest;
				sendValue["fileSize"] = (int)fileSize;
				sendValue["fileName"] = recvValue["fileName"].asString();
				SendJsonData(sendValue, clientSocket);
				SendFileDataToServer(fp, fileSize, clientSocket);
				fclose(fp);
				DebugLogUpdate(logBox, userName + recvValue["fileName"].asString() + " 파일전송성공");
			}
			else
				DebugLogUpdate(logBox, userName + recvValue["fileName"].asString() + " 파일전송실패");
			
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