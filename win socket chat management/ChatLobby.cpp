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
	COPYDATASTRUCT cds;
	Json::Value recvJson;
	//char* str = nullptr;

	while (true)
	{
		recvJson = Client::GetInstance()->RecvPacketToServer();
		if (recvJson == NULL)
			break;

		switch (recvJson["kind"].asInt())
		{
		case Message:
			// 채팅 다이얼로그로 받은 데이터 넘기는 코드 작성 해야 함
			//========================================================

			cds.dwData = 0;
			/*cds.lpData = recvJson["message"].asString().c_str();*/
			// 이쪽 데이터 세팅 부분 잘 처리해야 함..;;

			/*SendMessage(chattingDlgVector[recvJson["roomNumber"].asInt()].hwnd,
				WM_COPYDATA, NULL, (LPARAM)&cds);*/
			break;
		default:
			break;
		}
	}
	
	return 0;
}
