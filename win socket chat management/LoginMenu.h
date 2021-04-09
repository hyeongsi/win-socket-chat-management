#pragma once
#include <WinSock2.h>
#include <string>
#include "Client.h"

constexpr const int ipEditboxCharSize = 30;

using namespace std;

enum LoginFailed
{
	LoginSuccess = 0,
	WrongIdOrPassword = 1,
	Cancel = 2,
	Ban = 3,
};

class LoginMenu
{
private:
	static LoginMenu* instance;
	
	LoginMenu();
public:
	HINSTANCE g_hInst;
	bool isLogin;

	static LoginMenu* GetInstance();
	static void ReleaseInstance();

	static BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	
	void SetInst(HINSTANCE g_hInst);

	static void ClientLoginButton(HWND hDlg, WPARAM wParam);
	void ClientLoginBtnMethod(HWND hDlg, WPARAM wParam);
	Json::Value LoginCheck(string id, string pw);
	bool ConnectServer(string ip, string port);
};

