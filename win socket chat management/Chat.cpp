#include "Chat.h"
#include <string>
#include <atlbase.h>

using namespace std;

HWND chatDlgHandle;

BOOL CALLBACK ChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	COPYDATASTRUCT* pcds;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		chatDlgHandle = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COPYDATA:
		pcds = (PCOPYDATASTRUCT)lParam;
		MessageBox(hDlg, (LPCSTR)pcds->lpData, "test", 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SEND_BTN:
			SendMessageToServer(hDlg);
			break;
		}

		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void SendMessageToServer(HWND hwnd)
{
	char tempChatMessage[PACKET_SIZE];
	GetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), tempChatMessage, PACKET_SIZE);

	Client::GetInstance()->SendMessageToServer(tempChatMessage);

	SetWindowText(GetDlgItem(hwnd, IDC_EDIT_MESSAGEBOX), "");
}

void RecvJsonData(HWND hDlg, Json::Value value)
{
	if (chatDlgHandle != hDlg)
		return;

	string message = value["name"].asString() + " : " + value["message"].asString();
	const int substrSize = 40;
	int i = 0;

	SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
		(LPARAM)TEXT(
			(message).c_str()));

	// 특정 글자 수에 따라서 줄바꿈 구현 하려 하는데, 한글이 자를 때 짤리거나 하는 것 때문에
	// 잠시 보류, 찾아보니까 for문 돌려서 if(char & 0x80) 로 한글 확인해서 처리하던데.. 이해가 안됨. 일단 보류

	/*do
	{
		SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
			(LPARAM)TEXT(
				(message.substr(i * substrSize, substrSize)).c_str()));

		i++;
	} while (i < message.size() / substrSize);*/

}
