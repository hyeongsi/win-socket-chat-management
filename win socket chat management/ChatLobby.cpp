#include "ChatLobby.h"
#include <string>
#include <process.h>

using namespace std;

vector<chattingRoomHwnd> chattingDlgVector;

BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		// init 추가해야 함, 친구내역, 내 이름 세팅 작업
		//SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MYNAME), ""); 내이름 세팅
		SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_ADDSTRING, 0, (LPARAM)"메인 채팅방");
		_beginthreadex(NULL, 0, RecvMessageThread, NULL, 0, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST_FRIENDS:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				if (0 == SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETCURSEL, 0, 0))
				{
					chattingDlgVector.emplace_back(
						ChattingRoomHwnd(CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChatDlgProc)
							, 0));
					ShowWindow(chattingDlgVector.back().hwnd, SW_SHOW);
				}
				break;
			}
			break;
		case IDC_SENDTEST_BTN:
			Client::GetInstance()->SendMessageToServer("테스트입니다.");
			break;
		}

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

unsigned __stdcall RecvMessageThread(void* arg)
{
	Json::Value recvJson;

	while (true)
	{
		recvJson = Client::GetInstance()->RecvPacketToServer();
		if (recvJson == NULL)
			break;

		switch (recvJson["kind"].asInt())
		{
		case Message:
			RecvJsonData(chattingDlgVector[recvJson["roomNumber"].asInt()].hwnd, recvJson);
			break;
		default:
			break;
		}
	}
	
	return 0;
}
