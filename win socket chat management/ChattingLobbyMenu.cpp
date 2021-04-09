#include <process.h>
#include <sstream>
#include "ChattingLobbyMenu.h"
#include "resource.h"
#include "InputMenu.h"
#include "ChattingMenu.h"

ChattingLobbyMenu* ChattingLobbyMenu::instance = nullptr;
ChattingLobbyMenu::ChattingLobbyMenu() 
{
	g_hInst = nullptr;
	hChatLobbyDlg = nullptr;
};

ChattingLobbyMenu* ChattingLobbyMenu::GetInstance()
{
	if (nullptr == instance)
	{
		instance = new ChattingLobbyMenu();
	}

	return instance;
}

void ChattingLobbyMenu::ReleaseInstance()
{
	delete instance;
	instance = nullptr;
}

void ChattingLobbyMenu::SetInst(HINSTANCE g_hInst)
{
	GetInstance()->g_hInst = g_hInst;
}

BOOL ChattingLobbyMenu::ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
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
			AddFriendButton(hDlg);
			break;
		case IDC_LIST_FRIENDS:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				ClickChattingRoom(hDlg);
				break;
			}
			break;
		case IDC_ADD_CHATTING_ROOM_BTN:
			AddChattingRoomButton(hDlg);
			break;
		case IDC_FRIEND_BTN:
			FindFriendButton(hDlg);
			break;
		case IDC_CHAT_BTN2:
			FindChattingRoomButton(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void ChattingLobbyMenu::ChattingLobbyInit(HWND hDlg)
{
	GetInstance()->ChattingLobbyInitMethod(hDlg);
}

void ChattingLobbyMenu::ChattingLobbyInitMethod(HWND hDlg)
{
	Json::Value sendJson, recvJson;
	vector<string> chattingRoomNumber;
	string name;

	SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
	GetInstance()->hChatLobbyDlg = hDlg;

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

void ChattingLobbyMenu::FriendListInit(HWND hDlg)
{
	GetInstance()->FriendListInitMethod(hDlg);
}

void ChattingLobbyMenu::FriendListInitMethod(HWND hDlg)
{
	Json::Value sendJson, recvJson;
	vector<string> chattingRoomNumber;
	string name;

	//SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING, 0, (LPARAM)"메인 채팅방");

	sendJson["kind"] = GetFriendData;
	Client::GetInstance()->SendPacketToServer(sendJson);
}

void ChattingLobbyMenu::AddFriendButton(HWND hDlg)
{
	GetInstance()->AddFriendBtnMethod(hDlg);
}

void ChattingLobbyMenu::AddFriendBtnMethod(HWND hDlg)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_ID), hDlg, InputMenu::GetInstance()->InputIDDlgProc);

	if (strcmp(inputFriendId, "") == 0)
		return;

	Json::Value sendJson;

	sendJson["kind"] = AddFriend;

	sendJson["friendId"] = inputFriendId;
	Client::GetInstance()->SendPacketToServer(sendJson);

	strcpy_s(inputFriendId, PACKET_SIZE, "");
}

void ChattingLobbyMenu::ClickChattingRoom(HWND hDlg)
{
	GetInstance()->ClickChattingRoomMethod(hDlg);
}

void ChattingLobbyMenu::ClickChattingRoomMethod(HWND hDlg)
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
	ChattingMenu::GetInstance()->SetInst(g_hInst);
	chattingDlgVector[curSelNumber].hwnd =
		CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChattingMenu::GetInstance()->ChatDlgProc);
	chattingDlgVector[curSelNumber].turnOn = true;

	ShowWindow(chattingDlgVector[curSelNumber].hwnd, SW_SHOW);
	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETTEXT, curSelNumber, (LPARAM)roomName.c_str());
	SetWindowText(GetDlgItem(chattingDlgVector[curSelNumber].hwnd, IDC_STATIC_CHAT_ROOM_NAME)
		, roomName.c_str());
	chattingMutex.unlock();
	return;
}

void ChattingLobbyMenu::AddChattingRoomButton(HWND hDlg)
{
	GetInstance()->AddChattingRoomBtnMethod(hDlg);
}

void ChattingLobbyMenu::AddChattingRoomBtnMethod(HWND hDlg)
{
	Json::Value sendJson;

	sendJson["kind"] = AddChattingRoom;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_ROOMNAME), hDlg, InputMenu::GetInstance()->InputRoomNameDlgProc);

	if (string(inputRoomName) == "")
		return;

	sendJson["roomName"] = inputRoomName;
	Client::GetInstance()->SendPacketToServer(sendJson);
	strcpy_s(inputRoomName, PACKET_SIZE, "");
}

void ChattingLobbyMenu::FindFriendButton(HWND hDlg)
{
	GetInstance()->FindFriendBtnMethod(hDlg);
}

void ChattingLobbyMenu::FindFriendBtnMethod(HWND hDlg)
{
	if (isFriendList)
		return;

	isFriendList = true;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < (int)friendVector.size(); i++)
	{
		SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
			0, (LPARAM)friendVector[i].c_str());
	}
}

void ChattingLobbyMenu::FindChattingRoomButton(HWND hDlg)
{
	GetInstance()->FindChattingRoomBtnMethod(hDlg);
}

void ChattingLobbyMenu::FindChattingRoomBtnMethod(HWND hDlg)
{
	if (!isFriendList)
		return;

	isFriendList = false;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_RESETCONTENT, 0, 0);

	for (int i = 0; i < (int)chattingDlgVector.size(); i++)
	{
		SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
			0, (LPARAM)chattingDlgVector[i].roomName.c_str());
	}
}

vector<string> ChattingLobbyMenu::SplitString(string input, char delimiter)
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

unsigned __stdcall ChattingLobbyMenu::RecvMessageThread(void* arg)
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
			RecvMessage(recvJson);
			break;
		case SetFileRequest:
			if (Client::GetInstance()->RecvFileData(recvJson))
				MessageBox(GetInstance()->hChatLobbyDlg, "파일 저장 완료", ("downloadFiles\\" + recvJson["fileName"].asString()).c_str(), 0);
			break;
		case AddFriend:
			AddFriendMessage(recvJson);
			break;
		case AddChattingRoom:
			AddChattingRoomMessage(recvJson);
			break;
		case AddChattingRoomUser:
			AddChattingRoomUserMessage(recvJson);
			break;
		case GetFriendData:
			GetFriendDataMessage(recvJson);
			break;
		default:
			break;
		}
	}

	return 0;
}

void ChattingLobbyMenu::RecvMessage(Json::Value recvJson)
{
	GetInstance()->RecvMessageMethod(recvJson);
}

void ChattingLobbyMenu::RecvMessageMethod(Json::Value recvJson)
{
	chattingMutex.lock();
	for (const auto& iterator : chattingDlgVector)
	{
		if (iterator.roomNumber != recvJson["roomNumber"].asInt())
			continue;

		ChattingMenu::GetInstance()->SyncChatUI(iterator.hwnd, recvJson);
		break;
	}
	chattingMutex.unlock();
}

void ChattingLobbyMenu::AddFriendMessage(Json::Value recvJson)
{
	GetInstance()->AddFriendMessageMethod(recvJson);
}

void ChattingLobbyMenu::AddFriendMessageMethod(Json::Value recvJson)
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

void ChattingLobbyMenu::AddChattingRoomMessage(Json::Value recvJson)
{
	GetInstance()->AddChattingRoomMessageMethod(recvJson);
}

void ChattingLobbyMenu::AddChattingRoomMessageMethod(Json::Value recvJson)
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

void ChattingLobbyMenu::AddChattingRoomUserMessage(Json::Value recvJson)
{
	GetInstance()->AddChattingRoomUserMessageMethod(recvJson);
}

void ChattingLobbyMenu::AddChattingRoomUserMessageMethod(Json::Value recvJson)
{
	chattingMutex.lock();
	chattingDlgVector.emplace_back(
		chattingRoomHwnd(NULL, recvJson["roomNumber"].asInt(), recvJson["roomName"].asString()));
	SendMessage(GetDlgItem(hChatLobbyDlg, IDC_LIST_FRIENDS), LB_ADDSTRING,
		0, (LPARAM)recvJson["roomName"].asString().c_str());
	chattingMutex.unlock();
}

void ChattingLobbyMenu::GetFriendDataMessage(Json::Value recvJson)
{
	GetInstance()->GetFriendDataMessageMethod(recvJson);
}

void ChattingLobbyMenu::GetFriendDataMessageMethod(Json::Value recvJson)
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
