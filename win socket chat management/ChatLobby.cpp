#include "ChatLobby.h"
#include <string>
#include <process.h>

using namespace std;

vector<chattingRoomHwnd> chattingDlgVector;
vector<downLoadFileLine> downLoadFileLineVector;	// 파일 다운로드 라인 저장 변수
HWND hChatLobbyDlg;

BOOL CALLBACK ChatLobbyDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int curSelNumber = 0;
	bool isExistsRoom = false;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		// init 추가해야 함, 친구내역, 내 이름 세팅 작업
		// SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MYNAME), ""); 내이름 세팅
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
				isExistsRoom = false;
				curSelNumber = SendMessage(GetDlgItem(hDlg, IDC_LIST_FRIENDS), LB_GETCURSEL, 0, 0);

				// 몇번에있는게 방번호가 어떤건지 확인하는 로직 필요함. 일단은 0으로 설정
				// 로그인할 때, 친구들 id 가져와서 리스트로 가지고 있고, 그에따라 대화방아무튼 구현
				for (auto iterator = chattingDlgVector.begin(); iterator != chattingDlgVector.end();)
				{
					if ((*iterator).roomNumber == curSelNumber)
					{
						isExistsRoom = true;
						break;
					}
				}

				if (isExistsRoom)	// 방이 이미 존재하면 무시
					break;

				chattingDlgVector.emplace_back(
					ChattingRoomHwnd(CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_CHATTING), hDlg, ChatDlgProc)
						, 0));
				ShowWindow(chattingDlgVector.back().hwnd, SW_SHOW);
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
		case GetFileRequest:
			SyncChatUI(chattingDlgVector[recvJson["roomNumber"].asInt()].hwnd, recvJson);
			break;
		case SetFileRequest:
			if (Client::GetInstance()->RecvFileData(recvJson))
				MessageBox(hChatLobbyDlg, "파일 저장 완료",("downloadFiles\\" + recvJson["fileName"].asString()).c_str(),0);
			break;
		default:
			break;
		}
	}
	
	return 0;
}
