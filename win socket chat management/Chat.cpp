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

		// ����Ʈ�ڽ� ������ ���� �޽���, (���� ������ �޽���) ��ȣ�� ����ϰ� �ִٰ� ���߿� ���
		downLoadFileLineVector.emplace_back(hDlg, (SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0) + 1),
			value["fileName"].asString());
		break;
	}

	SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
		(LPARAM)TEXT(
			(message).c_str()));

	// Ư�� ���� ���� ���� �ٹٲ� ���� �Ϸ� �ϴµ�, �ѱ��� �ڸ� �� ©���ų� �ϴ� �� ������
	// ��� ����, ã�ƺ��ϱ� for�� ������ if(char & 0x80) �� �ѱ� Ȯ���ؼ� ó���ϴ���.. ���ذ� �ȵ�. �ϴ� ����

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
	// for ���Ƽ� ���߿� �ε��� �´°� ������, ������ ������ ������ ��û�Ѵ����� ���� �ޱ� ����,
	for (auto iterator = downLoadFileLineVector.begin(); iterator != downLoadFileLineVector.end();)
	{
		if (((*iterator).hwnd == hDlg) &&
			((*iterator).line == SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0)))
		{	// �α� Ŭ�� �� ���� ���ε� �޽����� Ŭ���ߴ��� üũ�ϰ�, �ش� �޽��� Ŭ�� �� ���� �ٿ�ε� ����
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
			// �ش� ���� �̸� �����޶�� ��û
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

	GetCurrentDirectory(256, curDirectoryPath);         // GetOpenFileName ȣ���ϸ� �⺻ ��θ��� �ٲ�� ������ �⺻ ��θ� �̸� ����

	ZeroMemory(&openFileName, sizeof(openFileName));    // ����ü�� 0���� �¾�
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.hwndOwner = hDlg;
	openFileName.lpstrTitle = "����÷��";
	openFileName.lpstrFileTitle = strFileTitle;
	openFileName.lpstrFile = strFilePath;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFilter = NULL;
	openFileName.nMaxFile = MAX_PATH;
	openFileName.nMaxFileTitle = MAX_PATH;

	if (GetOpenFileName(&openFileName) != 0)    // �ε����� 1���� �����ϱ� ������ ����
	{
		strcat_s(strFilePath, "\0");
		long fileSize;	// ���� ��ü ������
		FILE* fp;
		fopen_s(&fp, strFilePath, "rb");	// ���� ����
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
	SetCurrentDirectory(curDirectoryPath);  // ����� ��θ� ���� ��η� ����
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