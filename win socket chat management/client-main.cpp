#include "client-main.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, MainDlgProc);
	if(isLogin)
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), HWND_DESKTOP, ChatLobbyDlgProc);

	PostQuitMessage(0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_LOGIN_BTN:
			char ipStr[ipEditboxCharSize], portStr[ipEditboxCharSize];
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_IP), ipStr, ipEditboxCharSize);
			GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT), portStr, ipEditboxCharSize);

			switch (LoginCheck())
			{
			case NotFoundId:
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_ID), "");
				MessageBox(hDlg, "id 혹은 pw가 잘못되었습니다.", "로그인 오류", NULL);
				return false;
			case WrongPassword:
				SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PW), "");
				MessageBox(hDlg, "id 혹은 pw가 잘못되었습니다.", "로그인 오류", NULL);
				return false;
			}

			if (!ConnectServer(ipStr, portStr))
			{
				MessageBox(hDlg, "서버 연결 실패", "로그인 오류", NULL);
				return false;
			}

			isLogin = true;
			EndDialog(hDlg, wParam);
			break;
		case ID_GOTO_SIGNUP_BTN:
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_SIGNUP), hDlg, SignUpDlgProc);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}