#pragma
#include "ChatLobby.h"
#include <string>
#include <process.h>

using namespace std;

vector<chattingRoomHwnd> chattingDlgVector;
vector<downLoadFileLine> downLoadFileLineVector;	// 파일 다운로드 라인 저장 변수
vector<string> friendVector;
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
			for (const auto& iterator : chattingDlgVector)
			{
				if (iterator.roomNumber != recvJson["roomNumber"].asInt())
					continue;

				SyncChatUI(iterator.hwnd, recvJson);
				break;
			}
			break;
		case SetFileRequest:
			if (Client::GetInstance()->RecvFileData(recvJson))
				MessageBox(hChatLobbyDlg, "파일 저장 완료",("downloadFiles\\" + recvJson["fileName"].asString()).c_str(),0);
			break;
		case AddFriend:
			if (!recvJson["result"].asBool())
			{
				MessageBox(hChatLobbyDlg, recvJson["message"].asString().c_str(), "친구추가", 0);
				break;
			}

			friendVector.emplace_back(recvJson["friendId"].asString());
			MessageBox(hChatLobbyDlg, "친구추가 성공", "친구추가", 0);
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
	chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, 0));

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

			chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, stoi(chattingRoomNumber[0])));	// 리스트에 채팅방 번호 저장해서 관리
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

				chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, stoi(chattingRoomNumber[i])));	// 리스트에 채팅방 번호 저장해서 관리
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
	Json::Value sendJson;

	sendJson["kind"] = AddFriend;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_ID), hDlg, InputIDDlgProc);

	sendJson["friendId"] = inputFriendId;
	Client::GetInstance()->SendPacketToServer(sendJson);
}

void ClickChattingRoomMethod(HWND hDlg)
{
	int curSelNumber = 0;
	string roomName;

	curSelNumber = SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETCURSEL, 0, 0);

	if (chattingDlgVector[curSelNumber].turnOn)
		return;

	chattingDlgVector[curSelNumber].hwnd = 
		CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChatDlgProc);
	chattingDlgVector[curSelNumber].turnOn = true;
	
	ShowWindow(chattingDlgVector[curSelNumber].hwnd, SW_SHOW);
	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETTEXT, curSelNumber, (LPARAM)roomName.c_str());
	SetWindowText(GetDlgItem(chattingDlgVector[curSelNumber].hwnd, IDC_STATIC_CHAT_ROOM_NAME)
		, roomName.c_str());
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

void AddChattingRoomMethod(Json::Value recvJson)
{
	if (!recvJson["result"].asBool())
		return;

	chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, recvJson["roomNumber"].asInt()));
	SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
		0, (LPARAM)recvJson["roomName"].asString().c_str());

	Json::Value sendJson;
	sendJson["kind"] = AddChattingRoomUser;
	sendJson["addUserId"] = myId;
	sendJson["roomName"] = recvJson["roomName"].asString();
	sendJson["roomNumber"] = to_string(recvJson["roomNumber"].asInt());
	Client::GetInstance()->SendPacketToServer(sendJson);
}

void AddChattingRoomUserMethod(Json::Value recvJson)
{
	chattingDlgVector.emplace_back(chattingRoomHwnd(NULL, recvJson["roomNumber"].asInt()));
	SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
		0, (LPARAM)recvJson["roomName"].asString().c_str());
}

void GetFriendDataMethod(Json::Value recvJson)
{
	stringstream ss(recvJson["friend"].asString());
	string buffer;

	while (getline(ss, buffer, ','))
	{
		friendVector.emplace_back(buffer);
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