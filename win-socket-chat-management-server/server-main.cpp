#include "server-main.h"
#pragma comment(lib,"ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInst;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, MainDlgProc);

	PostQuitMessage(0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		g_hDlg = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);

		if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			MessageBox(g_hDlg, "WSAStartup Error", "error", NULL);
			return false;
		}

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_IP), GetMyIP().c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_PORT), "4567");
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_START_SERVER_BTN:
			AcceptThreadHandle = (HANDLE)_beginthreadex(NULL, 0, StartServer, NULL, 0, NULL);
			break;
		case ID_STOP_SERVER_BTN:
			StopServer();
			break;
		}
		break;
	case WM_CLOSE:
		closesocket(serverSocket);
		WSACleanup();
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}