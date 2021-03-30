#include "client-main.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, MainDlgProc);
	if (isLogin)
	{
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), HWND_DESKTOP, ChatLobbyDlgProc);
	}
		
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
			ClientLoginBtnMethod(hDlg, wParam);
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

void ClientLoginBtnMethod(HWND hDlg, WPARAM wParam)
{
	Json::Value loginValue;
	char tempStr[ipEditboxCharSize], tempStr2[ipEditboxCharSize];

	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_IP), tempStr, ipEditboxCharSize);
	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PORT), tempStr2, ipEditboxCharSize);

	if (!ConnectServer(tempStr, tempStr2))
	{
		MessageBox(hDlg, "서버 연결 실패", "로그인 오류", NULL);
		return;
	}

	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_ID), tempStr, ipEditboxCharSize);
	GetWindowText(GetDlgItem(hDlg, IDC_EDIT_PW), tempStr2, ipEditboxCharSize);

	loginValue = LoginCheck(tempStr, tempStr2);
	switch (loginValue["result"].asInt())
	{
	case WrongIdOrPassword:
		MessageBox(hDlg, loginValue["message"].asString().c_str(), "로그인 오류", NULL);
		return;
	case Cancel:
		MessageBox(hDlg, "서버 통신 오류", "로그인 오류", NULL);
		return;
	case Ban:
		MessageBox(hDlg, "정지된 계정입니다.", "계정 정지", NULL);
		return;
	}

	isLogin = true;
	EndDialog(hDlg, wParam);
}
