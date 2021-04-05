#include "Chat.h"
#include "ChatLobby.h"
#include <string>
#include <atlbase.h>

using namespace std;

HWND chatDlgHandle;
char inputFriendIdInChat[PACKET_SIZE];
extern vector<chattingRoomHwnd> chattingDlgVector;
extern vector<downLoadFileLine> downLoadFileLineVector;
extern vector<string> friendVector;
extern HWND hChatLobbyDlg;

BOOL CALLBACK ChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		chatDlgHandle = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST_CHAT_LOG:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				GetFileDataMethod(hDlg);
				break;
			}
			break;
		case IDC_SEND_BTN:
			SendMessageToServer(hDlg);
			break;
		case IDC_FILE_BTN:
			SendFileDataBtnMethod(hDlg);
			break;
		case IDC_ADD_USER_BTN:
			AddUserToChatBtnMethod(hDlg);
			break;
		}
		break;
	case WM_CLOSE:
		for (int i = 0; i < (int)chattingDlgVector.size(); i++)
		{
			if (hDlg == chattingDlgVector[i].hwnd)
			{
				chattingDlgVector[i].hwnd = NULL;
				chattingDlgVector[i].turnOn = false;
				break;
			}
		}
		
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void SendMessageToServer(HWND hwnd)
{
	char tempChatMessage[PACKET_SIZE];
	GetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), tempChatMessage, PACKET_SIZE);

	for (const auto& iterator : chattingDlgVector)
	{
		if (iterator.hwnd != hwnd)
			continue;

		Client::GetInstance()->SendMessageToServer(tempChatMessage, iterator.roomNumber);
		break;
	}

	SetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), "");
}

void SyncChatUI(HWND hDlg, Json::Value value)
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
		downLoadFileLineVector.emplace_back(hDlg, (SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0) + 1),
			value["fileName"].asString());
		break;
	}

	SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
		(LPARAM)TEXT(
			(message).c_str()));

	// 특정 글자 수에 따라서 줄바꿈 구현 하려 하는데, 한글이 자를 때 짤리거나 하는 것 때문에
	// 잠시 보류, 찾아보니까 for문 돌려서 if(char & 0x80) 로 한글 확인해서 처리하던데.. 이해가 안됨. 일단 보류

	/*do
	{
		SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
			(LPARAM)TEXT(
				(message.substr(i * substrSize, substrSize)).c_str()));

		i++;
	} while (i < message.size() / substrSize);*/

}

void GetFileDataMethod(HWND hDlg)
{
	// for 돌아서 값중에 인덱스 맞는게 있으면, 서버로 파일좀 보내줘 요청한다음에 파일 받기 실행,
	for (auto iterator = downLoadFileLineVector.begin(); iterator != downLoadFileLineVector.end();)
	{
		if (((*iterator).hwnd == hDlg) &&
			((*iterator).line == SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0)))
		{	// 로그 클릭 시 파일 업로드 메시지를 클릭했는지 체크하고, 해당 메시지 클릭 시 파일 다운로드 진행
			Json::Value value;
			value["kind"] = GetFileRequest;
			value["fileName"] = (*iterator).fileName;

			for (const auto& iterator : chattingDlgVector)
			{
				if (iterator.hwnd != hDlg)
					continue;

				value["roomNumber"] = iterator.roomNumber;
				break;
			}
			Client::GetInstance()->SendPacketToServer(value);
			// 해당 파일 이름 보내달라고 요청
			return;
		}
		else
		{
			iterator++;
		}
	}
	return;
}

void SendFileDataBtnMethod(HWND hDlg)
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

			for (const auto& iterator : chattingDlgVector)
			{
				if (iterator.hwnd != hDlg)
					continue;

				root["roomNumber"] = iterator.roomNumber;
				break;
			}

			Client::GetInstance()->SendPacketToServer(root);
			Client::GetInstance()->SendFileDataToServer(fp, fileSize);

			fclose(fp);
		}
	}
	SetCurrentDirectory(curDirectoryPath);  // 변경된 경로를 원래 경로로 설정
}

void AddUserToChatBtnMethod(HWND hDlg)
{
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_ADD_CHAT_USER), hDlg, AddFriendInChatDlgProc);
	if (strcmp(inputFriendIdInChat, "") == 0)
		return;

	char roomName[PACKET_SIZE];

	Json::Value sendValue;
	sendValue["kind"] = AddChattingRoomUser;
	sendValue["addUserId"] = inputFriendIdInChat;
	GetWindowText(GetDlgItem(hDlg, IDC_STATIC_CHAT_ROOM_NAME), (LPSTR)roomName, PACKET_SIZE);
	sendValue["roomName"] = roomName;
	
	for (const auto& iterator : chattingDlgVector)
	{
		if (hDlg == iterator.hwnd)
		{
			sendValue["roomNumber"] = to_string(iterator.roomNumber);
			break;
		}
	}
	Client::GetInstance()->SendPacketToServer(sendValue);

	strcpy_s(inputFriendIdInChat, PACKET_SIZE, "");
}

BOOL CALLBACK AddFriendInChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		for (auto myFriend : friendVector)
		{
			SendMessage(GetDlgItem(hDlg, IDC_ADD_FRIEND_LIST), LB_ADDSTRING, 0,
				(LPARAM)TEXT(
					myFriend.c_str()));
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hDlg, IDC_ADD_FRIEND_LIST), LB_GETTEXT,
				SendMessage(GetDlgItem(hDlg, IDC_ADD_FRIEND_LIST), LB_GETCURSEL, 0, 0), (LPARAM)inputFriendIdInChat);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			strcpy_s(inputFriendIdInChat, PACKET_SIZE, "");
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