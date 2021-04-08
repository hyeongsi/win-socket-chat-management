#include "server-main.h"
#include <process.h>
#pragma comment(lib,"ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, MainDlgProc);

	ChattingRoomManager::GetInstance();
	MembershipDB::ReleaseInstance();
	Server::ReleaseInstance();
	ServerUI::ReleaseInstance();
	PostQuitMessage(0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		ServerUI::GetInstance()->InitDialogMethod(hDlg);	// ip, port, hDlg, UI 생성 위치 설정
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONNECT_USER_CHECK_BTN:	// 접속사 수 확인
			ServerUI::GetInstance()->CheckConnectUserBtnMethod();
			break;
		case ID_USER_CHECK_BTN:			// 모든 사용자 확인
			ServerUI::GetInstance()->CheckUserIdListBtnMethod();
			break;
		case ID_BAN_BUTTON:	// 계정 정지
			ServerUI::GetInstance()->BanBtnMethod();
			break;
		case ID_UNBAN_BUTTON:
			ServerUI::GetInstance()->UnBanBtnMethod();
			break;
		case ID_SERVER_SAVE_LOG_BTN:
			ServerUI::GetInstance()->SaveServerLogBtnMethod();
			break;
		case ID_START_SERVER_BTN:
			_beginthreadex(NULL, 0, Server::GetInstance()->StartServer, NULL, 0, NULL);
			break;
		case ID_STOP_SERVER_BTN:
			Server::GetInstance()->StopServer();
			break;
		}
		break;
	case WM_CLOSE:
		Server::GetInstance()->CloseServerSocket();
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}