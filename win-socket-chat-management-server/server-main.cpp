#include <WinSock2.h>
#include "resource.h"

#pragma comment(lib,"ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

SOCKET serverSocket;
SOCKADDR_IN serverAddress = {};

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
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);

		serverSocket;
		serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);	// 통신 프로토콜 설정

		serverAddress = {};
		serverAddress.sin_family = AF_INET;
		// sin_familly : 소켓 주소 체계
		serverAddress.sin_port = htons(4567);
		// sin_port : 포트
		serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);	// s_addr : IPv4 의미, INADDR_ANY : 현재 동작되는 컴퓨터의 IP주소
		// sin_addr : IP 주소

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_IP), inet_ntoa(serverAddress.sin_addr));
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_PORT), "4567");
		break;
	case WM_COMMAND:
		/*switch (LOWORD(wParam))
		{

		}
		return FALSE;*/

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}