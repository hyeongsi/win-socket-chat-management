#include "ChatLobby.h"

BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HWND chatDialogHandle;
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SENDTEST_BTN:
			Client::GetInstance()->SendMessageToServer("테스트입니다.");
			break;
		case IDC_ROOMTEST_BTN:
			chatDialogHandle = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChatDlgProc);
			ShowWindow(chatDialogHandle, SW_SHOW);
			break;
		}

		break;
		return FALSE;

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}