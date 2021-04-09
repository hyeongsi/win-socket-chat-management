#pragma once

#include "LoginMenu.h"
#include "ChattingLobbyMenu.h"
#include "resource.h"

#pragma comment(lib, "ws2_32")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	LoginMenu::GetInstance()->SetInst(hInstance);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, LoginMenu::GetInstance()->MainDlgProc);
	if (LoginMenu::GetInstance()->isLogin)
	{
		ChattingLobbyMenu::GetInstance()->SetInst(hInstance);
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), HWND_DESKTOP, ChattingLobbyMenu::GetInstance()->ChatLobbyDlgProc);
	}
		
	LoginMenu::ReleaseInstance();
	ChattingLobbyMenu::ReleaseInstance();
	PostQuitMessage(0);
	return 0;
}