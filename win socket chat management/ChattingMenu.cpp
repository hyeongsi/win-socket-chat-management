#include "ChattingMenu.h"
#include "resource.h"
#include "Client.h"
#include "ChattingLobbyMenu.h"
#include "InputMenu.h"

ChattingMenu* ChattingMenu::instance = nullptr;
ChattingMenu::ChattingMenu() 
{
	chatDlgHandle = nullptr;
	g_hInst = nullptr;
};

ChattingMenu* ChattingMenu::GetInstance()
{
	if (nullptr == instance)
	{
		instance = new ChattingMenu();
	}

	return instance;
}

void ChattingMenu::ReleaseInstance()
{
	delete instance;
	instance = nullptr;
}

void ChattingMenu::SetInst(HINSTANCE g_hInst)
{
	this->g_hInst = g_hInst;
}

BOOL ChattingMenu::ChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		GetInstance()->chatDlgHandle = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST_CHAT_LOG:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				GetFileDataMessage(hDlg);
				break;
			}
			break;
		case IDC_SEND_BTN:
			SendMessageToServer(hDlg);
			break;
		case IDC_FILE_BTN:
			SendFileDataButton(hDlg);
			break;
		case IDC_ADD_USER_BTN:
			AddUserToChatButton(hDlg);
			break;
		case IDC_EMOTICON_BTN:
			EmoticonButton(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		ChattingLobbyMenu::GetInstance()->chattingMutex.lock();
		for (int i = 0; i < (int)ChattingLobbyMenu::GetInstance()->chattingDlgVector.size(); i++)
		{
			if (hDlg == ChattingLobbyMenu::GetInstance()->chattingDlgVector[i].hwnd)
			{
				ChattingLobbyMenu::GetInstance()->chattingDlgVector[i].hwnd = NULL;
				ChattingLobbyMenu::GetInstance()->chattingDlgVector[i].turnOn = false;
				break;
			}
		}
		ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void ChattingMenu::GetFileDataMessage(HWND hDlg)
{
	GetInstance()->GetFileDataMessageMethod(hDlg);
}

void ChattingMenu::GetFileDataMessageMethod(HWND hDlg)
{
	ChattingLobbyMenu::GetInstance()->chattingMutex.lock();

	Json::Value value;
	bool isCollectRoom = false;

	for (auto fileIterator = ChattingLobbyMenu::GetInstance()->downLoadFileLineVector.begin();
		fileIterator != ChattingLobbyMenu::GetInstance()->downLoadFileLineVector.end();)
	{
		for (const auto& iterator : ChattingLobbyMenu::GetInstance()->chattingDlgVector)
		{
			if (iterator.hwnd != hDlg)
				continue;

			if ((*fileIterator).roomNumber == iterator.roomNumber)
			{
				isCollectRoom = true;
				value["roomNumber"] = iterator.roomNumber;
				break;
			}
		}

		if (!isCollectRoom)
		{
			ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();
			return;
		}

		if (
			((*fileIterator).line - 1) == SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCURSEL, 0, 0)
			)
		{	// 로그 클릭 시 파일 업로드 메시지를 클릭했는지 체크하고, 해당 메시지 클릭 시 파일 다운로드 진행
			Json::Value value;
			value["kind"] = GetFileRequest;
			value["fileName"] = (*fileIterator).fileName;

			ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();
			Client::GetInstance()->SendPacketToServer(value);
			// 해당 파일 이름 보내달라고 요청
			return;
		}
		else
		{
			fileIterator++;
		}
	}
	ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();
	return;
}

void ChattingMenu::SendMessageToServer(HWND hwnd)
{
	GetInstance()->SendMessageToServerMethod(hwnd);
}

void ChattingMenu::SendMessageToServerMethod(HWND hwnd)
{
	char tempChatMessage[PACKET_SIZE];
	GetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), tempChatMessage, PACKET_SIZE);

	ChattingLobbyMenu::GetInstance()->chattingMutex.lock();
	for (const auto& iterator : ChattingLobbyMenu::GetInstance()->chattingDlgVector)
	{
		if (iterator.hwnd != hwnd)
			continue;

		Client::GetInstance()->SendMessageToServer(tempChatMessage, iterator.roomNumber);
		break;
	}
	ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();

	SetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), "");
}

void ChattingMenu::SendFileDataButton(HWND hDlg)
{
	GetInstance()->SendFileDataBtnMethod(hDlg);
}

void ChattingMenu::SendFileDataBtnMethod(HWND hDlg)
{
	TCHAR curDirectoryPath[256];
	OPENFILENAME openFileName;
	static char strFileTitle[MAX_PATH], strFileExtension[10], strFilePath[100];

	GetCurrentDirectory(256, curDirectoryPath);         // GetOpenFileName 호출하면 기본 경로명이 바뀌기 때문에 기본 경로명 미리 저장

	ZeroMemory(&openFileName, sizeof(openFileName));    // 구조체를 0으로 셋업
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.hwndOwner = hDlg;
	openFileName.lpstrTitle = "파일첨부";
	openFileName.lpstrFileTitle = strFileTitle;
	openFileName.lpstrFile = strFilePath;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFilter = NULL;
	openFileName.nMaxFile = MAX_PATH;
	openFileName.nMaxFileTitle = MAX_PATH;

	if (GetOpenFileName(&openFileName) != 0)    // 인덱스가 1부터 시작하기 때문에 지정
	{
		strcat_s(strFilePath, "\0");
		long fileSize;	// 파일 전체 사이즈
		FILE* fp;
		fopen_s(&fp, strFilePath, "rb");	// 파일 열고
		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			fileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);

			Json::Value root;
			root["kind"] = SetFileRequest;
			root["fileSize"] = (int)fileSize;
			root["fileName"] = strFileTitle;

			ChattingLobbyMenu::GetInstance()->chattingMutex.lock();
			for (const auto& iterator : ChattingLobbyMenu::GetInstance()->chattingDlgVector)
			{
				if (iterator.hwnd != hDlg)
					continue;

				root["roomNumber"] = iterator.roomNumber;
				break;
			}
			ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();

			Client::GetInstance()->SendPacketToServer(root);
			Client::GetInstance()->SendFileDataToServer(fp, fileSize);

			fclose(fp);
		}
	}
	SetCurrentDirectory(curDirectoryPath);  // 변경된 경로를 원래 경로로 설정
}

void ChattingMenu::AddUserToChatButton(HWND hDlg)
{
	GetInstance()->AddUserToChatBtnMethod(hDlg);
}

void ChattingMenu::AddUserToChatBtnMethod(HWND hDlg)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_ADD_CHAT_USER), hDlg, InputMenu::GetInstance()->AddFriendInChatDlgProc);
	if (strcmp(InputMenu::GetInstance()->inputFriendIdInChat, "") == 0)
		return;

	char roomName[PACKET_SIZE];

	Json::Value sendValue;
	sendValue["kind"] = AddChattingRoomUser;
	sendValue["addUserId"] = InputMenu::GetInstance()->inputFriendIdInChat;
	GetWindowText(GetDlgItem(hDlg, IDC_STATIC_CHAT_ROOM_NAME), (LPSTR)roomName, PACKET_SIZE);
	sendValue["roomName"] = roomName;

	ChattingLobbyMenu::GetInstance()->chattingMutex.lock();
	for (const auto& iterator : ChattingLobbyMenu::GetInstance()->chattingDlgVector)
	{
		if (hDlg == iterator.hwnd)
		{
			sendValue["roomNumber"] = to_string(iterator.roomNumber);
			break;
		}
	}
	ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();
	Client::GetInstance()->SendPacketToServer(sendValue);

	strcpy_s(InputMenu::GetInstance()->inputFriendIdInChat, PACKET_SIZE, "");
}

void ChattingMenu::EmoticonButton(HWND hDlg)
{
	GetInstance()->EmoticonBtnMethod(hDlg);
}

void ChattingMenu::EmoticonBtnMethod(HWND hDlg)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUT_EMOTICON), hDlg, InputMenu::GetInstance()->InputEmoticonDlgProc);
	if (-1 == InputMenu::GetInstance()->selectEmoticon)
		return;

	Json::Value sendValue;
	sendValue["kind"] = Emoticon;
	sendValue["emoticon"] = InputMenu::GetInstance()->selectEmoticon;

	ChattingLobbyMenu::GetInstance()->chattingMutex.lock();
	for (const auto& iterator : ChattingLobbyMenu::GetInstance()->chattingDlgVector)
	{
		if (iterator.hwnd != hDlg)
			continue;

		sendValue["roomNumber"] = iterator.roomNumber;
		break;
	}
	ChattingLobbyMenu::GetInstance()->chattingMutex.unlock();

	Client::GetInstance()->SendPacketToServer(sendValue);
}

void ChattingMenu::SyncChatUI(HWND hDlg, Json::Value value)
{
	if (NULL == hDlg)
		return;

	string message;
	switch (value["kind"].asInt())
	{
	case Message:
		message = value["name"].asString() + " : " + value["message"].asString();
		break;
	case GetFileRequest:
		message = value["message"].asString();

		// 리스트박스 파일을 보낸 메시지, (제일 마지막 메시지) 번호를 기억하고 있다가 나중에 사용
		ChattingLobbyMenu::GetInstance()->downLoadFileLineVector.emplace_back((SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0) + 1),
			value["fileName"].asString(), value["roomNumber"].asInt());
		break;
	}

	int count = 0;
	for (int i = 0; message.c_str()[i]; i++)
	{
		if (message.c_str()[i] >> 7)
			i++;
		count++;
	}

	const int subStrSize = 20;
	string substrString;
	if ((count > subStrSize) && (value["kind"].asInt() != GetFileRequest))
	{
		for (int i = 0; message.c_str()[i]; i++)
		{
			if (message.c_str()[i] >> 7)
			{
				substrString += message.c_str()[i];
				i++;
			}

			substrString += message.c_str()[i];

			if (((substrString.size() / subStrSize) > 1) || (!message.c_str()[i + 1]))
			{
				SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
					(LPARAM)TEXT(
						substrString.c_str()));

				substrString.clear();
			}
		}
	}
	else
	{
		SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
			(LPARAM)TEXT(
				(message).c_str()));
	}
}

