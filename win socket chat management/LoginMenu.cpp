#include "LoginMenu.h"
#include "SignUpMenu.h"
#include "resource.h"

LoginMenu* LoginMenu::instance = nullptr;
LoginMenu::LoginMenu() 
{ 
	g_hInst = nullptr;
	isLogin = false;
}

LoginMenu* LoginMenu::GetInstance()
{
	if (nullptr == instance)
	{
		instance = new LoginMenu();
	}

	return instance;
}

void LoginMenu::ReleaseInstance()
{
	delete instance;
	instance = nullptr;
}

BOOL LoginMenu::MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		Client::GetInstance()->g_hDlg = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_LOGIN_BTN:
			ClientLoginButton(hDlg, wParam);
			break;
		case ID_GOTO_SIGNUP_BTN:
			DialogBox(GetInstance()->g_hInst, MAKEINTRESOURCE(IDD_DIALOG_SIGNUP), hDlg, SignUpMenu::GetInstance()->SignUpDlgProc);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void LoginMenu::SetInst(HINSTANCE g_hInst)
{
	this->g_hInst = g_hInst;
}

void LoginMenu::ClientLoginButton(HWND hDlg, WPARAM wParam)
{
	GetInstance()->ClientLoginBtnMethod(hDlg, wParam);
}

void LoginMenu::ClientLoginBtnMethod(HWND hDlg, WPARAM wParam)
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

Json::Value LoginMenu::LoginCheck(string id, string pw)
{
	Client::GetInstance()->SendLoginSignToServer(id, pw);

	Json::Value recvValue = Client::GetInstance()->RecvPacketToServer();
	if (recvValue == NULL)
		return Cancel;

	return recvValue;
}

bool LoginMenu::ConnectServer(string ip, string port)
{
	Client::GetInstance()->CloseSocket();	// 기존 연결 끊고 다시 연결 진행

	try
	{
		if (!(Client::GetInstance()->ConnectInit(ip, stoi(port))))	// 연결 실패 시
			return false;
	}
	catch (const std::exception& a)
	{
		printf("%s", a.what());
		return false;
	}

	return true;
}
