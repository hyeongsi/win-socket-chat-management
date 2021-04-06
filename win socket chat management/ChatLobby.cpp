#pragma
#include "ChatLobby.h"
#include <string>
#include <process.h>

using namespace std;

mutex chattingMutex;
vector<chattingRoomHwnd> chattingDlgVector;
vector<downLoadFileLine> downLoadFileLineVector;	// 파일 다운로드 라인 저장 변수
vector<string> friendVector;
bool isFriendList = false;
HWND hChatLobbyDlg;
char inputFriendId[PACKET_SIZE];
char inputRoomName[PACKET_SIZE];
string myId;

BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		ChattingLobbyInit(hDlg);
		FriendListInit(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADDFRIEND_BTN:
			AddFriendBtnMethod(hDlg);
			break;
		case IDC_LIST_FRIENDS:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				ClickChattingRoomMethod(hDlg);
				break;
			}
			break;
		case IDC_ADD_CHATTING_ROOM_BTN:
			AddChattingRoomBtnMethod(hDlg);
			break;
		case IDC_FRIEND_BTN:
			FriendBtnMethod(hDlg);
			break;
		case IDC_CHAT_BTN2:
			ChatBtnMethod(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

unsigned __stdcall RecvMessageThread(void* arg)
{
	Json::Value recvJson;

	while (true)
	{
		recvJson = Client::GetInstance()->RecvPacketToServer();
		if (recvJson == NULL)
			break;

		switch (recvJson["kind"].asInt())
		{
		case Message:
		case GetFileRequest:
			RecvMessageMethod(recvJson);
			break;
		case SetFileRequest:
			if (Client::GetInstance()->RecvFileData(recvJson))
				MessageBox(hChatLobbyDlg, "파일 저장 완료",("downloadFiles\\" + recvJson["fileName"].asString()).c_str(),0);
			break;
		case AddFriend:
			AddFriendMethod(recvJson);
			break;
		case AddChattingRoom:
			AddChattingRoomMethod(recvJson);
			break;
		case AddChattingRoomUser:
			AddChattingRoomUserMethod(recvJson);
			break;
		case GetFriendData:
			GetFriendDataMethod(recvJson);
			break;
		default:
			break;
		}
	}
	
	return 0;
}

vector<string> SplitString(string input, char delimiter)
{
	vector<string> row;
	stringstream ss(input);
	string buffer;

	while (getline(ss, buffer, delimiter))
	{
		row.emplace_back(buffer);
	}

	return row;
}

void ChattingLobbyInit(HWND hDlg)
{
	Json::Value sendJson, recvJson;
	vector<string> chattingRoomNumber;
	string name;

	SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
	hChatLobbyDlg = hDlg;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING, 0, (LPARAM)"메인 채팅방");

	chattingMutex.lock();
	chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, 0, "메인 채팅방"));
	chattingMutex.unlock();

	sendJson["kind"] = ChattingRoomInit;
	Client::GetInstance()->SendPacketToServer(sendJson);
	recvJson = Client::GetInstance()->RecvPacketToServer();
	name = recvJson["name"].asString();
	myId = recvJson["id"].asString();

	if (recvJson["roomNumberStr"].asString() != "")
	{
		chattingRoomNumber = SplitString(recvJson["roomNumberStr"].asString(), ',');

		sendJson["kind"] = GetChattringRoomName;
		if (chattingRoomNumber.size() <= 0)	// 채팅방 1개일 경우
		{
			sendJson["roomNumber"] = recvJson["roomNumberStr"].asString();

			Client::GetInstance()->SendPacketToServer(sendJson);
			recvJson = Client::GetInstance()->RecvPacketToServer();
			if (recvJson == NULL)
				return;

			SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
				0, (LPARAM)recvJson["roomName"].asString().c_str());
			chattingMutex.lock();
			chattingDlgVector.emplace_back(
				chattingRoomHwnd(NULL, stoi(chattingRoomNumber[0]), recvJson["roomName"].asString()));
			chattingMutex.unlock();// 리스트에 채팅방 번호 저장해서 관리
		}
		else //채팅방 n개 이상 경우
		{
			for (int i = 0; i < (int)chattingRoomNumber.size(); i++)
			{
				sendJson["roomNumber"] = chattingRoomNumber[i];
				Client::GetInstance()->SendPacketToServer(sendJson);
				recvJson = Client::GetInstance()->RecvPacketToServer();
				if (recvJson == NULL)
					return;

				SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
					0, (LPARAM)recvJson["roomName"].asString().c_str());
				chattingMutex.lock();
				chattingDlgVector.emplace_back(
					chattingRoomHwnd(NULL, stoi(chattingRoomNumber[i]), recvJson["roomName"].asString()));
				chattingMutex.unlock();// 리스트에 채팅방 번호 저장해서 관리
			}
		}
	}

	SetDlgItemText(hDlg, IDC_STATIC_MYNAME, name.c_str());
	_beginthreadex(NULL, 0, RecvMessageThread, NULL, 0, NULL);
}

void FriendListInit(HWND hDlg)
{
	Json::Value sendJson, recvJson;
	vector<string> chattingRoomNumber;
	string name;

	//SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING, 0, (LPARAM)"메인 채팅방");

	sendJson["kind"] = GetFriendData;
	Client::GetInstance()->SendPacketToServer(sendJson);
}

void AddFriendBtnMethod(HWND hDlg)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_ID), hDlg, InputIDDlgProc);

	if (strcmp(inputFriendId, "") == 0)
		return;

	Json::Value sendJson;

	sendJson["kind"] = AddFriend;

	sendJson["friendId"] = inputFriendId;
	Client::GetInstance()->SendPacketToServer(sendJson);

	strcpy_s(inputFriendId, PACKET_SIZE, "");
}

void ClickChattingRoomMethod(HWND hDlg)
{
	if (isFriendList)
		return;

	int curSelNumber = 0;
	string roomName;

	curSelNumber = SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETCURSEL, 0, 0);
	chattingMutex.lock();
	if (chattingDlgVector[curSelNumber].turnOn)
	{
		chattingMutex.unlock();
		return;
	}
	chattingMutex.unlock();

	chattingMutex.lock();
	chattingDlgVector[curSelNumber].hwnd = 
		CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChatDlgProc);
	chattingDlgVector[curSelNumber].turnOn = true;
	
	ShowWindow(chattingDlgVector[curSelNumber].hwnd, SW_SHOW);
	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETTEXT, curSelNumber, (LPARAM)roomName.c_str());
	SetWindowText(GetDlgItem(chattingDlgVector[curSelNumber].hwnd, IDC_STATIC_CHAT_ROOM_NAME)
		, roomName.c_str());
	chattingMutex.unlock();
	return;
}

void AddChattingRoomBtnMethod(HWND hDlg)
{
	Json::Value sendJson;

	sendJson["kind"] = AddChattingRoom;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_ROOMNAME), hDlg, InputRoomNameDlgProc);

	if (string(inputRoomName) == "")
		return;

	sendJson["roomName"] = inputRoomName;
	Client::GetInstance()->SendPacketToServer(sendJson);
	strcpy_s(inputRoomName, PACKET_SIZE, "");
}

void FriendBtnMethod(HWND hDlg)
{
	if (isFriendList)
		return;

	isFriendList = true;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < friendVector.size(); i++)
	{
		SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
			0, (LPARAM)friendVector[i].c_str());
	}
}

void ChatBtnMethod(HWND hDlg)
{
	if (!isFriendList)
		return;

	isFriendList = false;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < chattingDlgVector.size(); i++)
	{
		SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
			0, (LPARAM)chattingDlgVector[i].roomName.c_str());
	}
}

void AddChattingRoomMethod(Json::Value recvJson)
{
	if (!recvJson["result"].asBool())
		return;

	chattingMutex.lock();
	chattingDlgVector.emplace_back(
		chattingRoomHwnd(NULL, recvJson["roomNumber"].asInt(), recvJson["roomName"].asString()));
	SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
		0, (LPARAM)recvJson["roomName"].asString().c_str());
	chattingMutex.unlock();

	Json::Value sendJson;
	sendJson["kind"] = AddChattingRoomUser;
	sendJson["addUserId"] = myId;
	sendJson["roomName"] = recvJson["roomName"].asString();
	sendJson["roomNumber"] = to_string(recvJson["roomNumber"].asInt());
	Client::GetInstance()->SendPacketToServer(sendJson);
}

void AddChattingRoomUserMethod(Json::Value recvJson)
{
	chattingMutex.lock();
	chattingDlgVector.emplace_back(
		chattingRoomHwnd(NULL, recvJson["roomNumber"].asInt(), recvJson["roomName"].asString()));
	SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
		0, (LPARAM)recvJson["roomName"].asString().c_str());
	chattingMutex.unlock();
}

void GetFriendDataMethod(Json::Value recvJson)
{
	stringstream ss(recvJson["friend"].asString());
	string buffer;

	chattingMutex.lock();
	while (getline(ss, buffer, ','))
	{
		friendVector.emplace_back(buffer);
	}
	chattingMutex.unlock();
}

void RecvMessageMethod(Json::Value recvJson)
{
	chattingMutex.lock();
	for (const auto& iterator : chattingDlgVector)
	{
		if (iterator.roomNumber != recvJson["roomNumber"].asInt())
			continue;

		SyncChatUI(iterator.hwnd, recvJson);
		break;
	}
	chattingMutex.unlock();
}

void AddFriendMethod(Json::Value recvJson)
{
	if (!recvJson["result"].asBool())
	{
		MessageBox(hChatLobbyDlg, recvJson["message"].asString().c_str(), "친구추가", 0);
		return;
	}

	chattingMutex.lock();
	friendVector.emplace_back(recvJson["friendId"].asString());
	chattingMutex.unlock();
	MessageBox(hChatLobbyDlg, "친구추가 성공", "친구추가", 0);

	if (isFriendList)
	{
		SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING, 0, (LPARAM)recvJson["friendId"].asString().c_str());
	}
}

BOOL CALLBACK InputIDDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_INPUT_ID), (LPSTR)inputFriendId, PACKET_SIZE);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			strcpy_s(inputFriendId, PACKET_SIZE, "");
			EndDialog(hDlg, wParam);
			return FALSE;
		}

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK InputRoomNameDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_INPUT_ROOMNAME), (LPSTR)inputRoomName, PACKET_SIZE);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			strcpy_s(inputRoomName, PACKET_SIZE, "");
			EndDialog(hDlg, wParam);
			return FALSE;
		}

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}