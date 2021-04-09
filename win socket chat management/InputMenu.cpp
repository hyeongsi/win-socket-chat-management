#include "InputMenu.h"
#include "resource.h"
#include "ChattingLobbyMenu.h"

InputMenu* InputMenu::instance = nullptr;
InputMenu::InputMenu() {};

InputMenu* InputMenu::GetInstance()
{
	if (nullptr == instance)
	{
		instance = new InputMenu();
		SetSelectEmoticon(-1);
	}

	return instance;
}

void InputMenu::ReleaseInstance()
{
	delete instance;
	instance = nullptr;
}

BOOL InputMenu::InputIDDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_INPUT_ID), (LPSTR)ChattingLobbyMenu::GetInstance()->inputFriendId, PACKET_SIZE);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			strcpy_s(ChattingLobbyMenu::GetInstance()->inputFriendId, PACKET_SIZE, "");
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

BOOL InputMenu::InputRoomNameDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_INPUT_ROOMNAME), (LPSTR)ChattingLobbyMenu::GetInstance()->inputRoomName, PACKET_SIZE);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			strcpy_s(ChattingLobbyMenu::GetInstance()->inputRoomName, PACKET_SIZE, "");
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

BOOL InputMenu::AddFriendInChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		for (auto myFriend : ChattingLobbyMenu::GetInstance()->friendVector)
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
				SendMessage(GetDlgItem(hDlg, IDC_ADD_FRIEND_LIST), LB_GETCURSEL, 0, 0), (LPARAM)GetInstance()->inputFriendIdInChat);
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			SetInputFriendIdInChat("");
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

BOOL InputMenu::InputEmoticonDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_EMOTICON_LIST), LB_ADDSTRING, 0,
			(LPARAM)"happy");
		SendMessage(GetDlgItem(hDlg, IDC_EMOTICON_LIST), LB_ADDSTRING, 0,
			(LPARAM)"sad");
		SendMessage(GetDlgItem(hDlg, IDC_EMOTICON_LIST), LB_ADDSTRING, 0,
			(LPARAM)"surprised");
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SetSelectEmoticon(SendMessage(GetDlgItem(hDlg, IDC_EMOTICON_LIST), LB_GETCURSEL, 0, 0));
			EndDialog(hDlg, wParam);
			return TRUE;
		case IDCANCEL:
			SetSelectEmoticon(-1);
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

void InputMenu::SetInputFriendIdInChat(const char* str)
{
	strcpy_s(GetInstance()->inputFriendIdInChat, PACKET_SIZE, str);
}

void InputMenu::SetSelectEmoticon(int selectEmoticon)
{
	GetInstance()->selectEmoticon = selectEmoticon;
}
