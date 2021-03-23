#include "Chat.h"
#include <string>
#include <atlbase.h>

using namespace std;

HWND chatDlgHandle;

BOOL CALLBACK ChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME openFileName;
	static char strFileTitle[MAX_PATH], strFileExtension[10], strFilePath[100];
	TCHAR curDirectoryPath[256];

	switch (iMessage)
	{
	case WM_INITDIALOG:
		chatDlgHandle = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SEND_BTN:
			SendMessageToServer(hDlg);
			break;
		case IDC_FILE_BTN:
			GetCurrentDirectory(256, curDirectoryPath);         // GetOpenFileName ȣ���ϸ� �⺻ ��θ��� �ٲ�� ������ �⺻ ��θ� �̸� ����

			ZeroMemory(&openFileName, sizeof(openFileName));    // ����ü�� 0���� �¾�
			openFileName.lStructSize = sizeof(openFileName);
			openFileName.hwndOwner = hDlg;
			openFileName.lpstrTitle = "����÷��";
			openFileName.lpstrFileTitle = strFileTitle;
			openFileName.lpstrFile = strFilePath;
			openFileName.nFilterIndex = 1;
			openFileName.lpstrFilter = NULL;
			openFileName.nMaxFile = MAX_PATH;
			openFileName.nMaxFileTitle = MAX_PATH;

			if (GetOpenFileName(&openFileName) != 0)    // �ε����� 1���� �����ϱ� ������ ����
			{
				strcat(strFilePath, "\0");
				long fileSize;	// ���� ��ü ������
				FILE* fp;
				fopen_s(&fp, strFilePath, "rb");	// ���� ����
				if (fp != NULL)
				{
					fseek(fp, 0, SEEK_END);
					fileSize = ftell(fp);
					fseek(fp, 0, SEEK_SET);

					Json::Value root;
					root["kind"] = Files;
					root["fileSize"] = (int)fileSize;
					root["fileName"] = strFileTitle;
					Client::GetInstance()->SendPacketToServer(root);
					Client::GetInstance()->SendFileDataToServer(fp, fileSize);

					fclose(fp);
				}
			}
			SetCurrentDirectory(curDirectoryPath);  // ����� ��θ� ���� ��η� ����
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

	// Ư�� ���� ���� ���� �ٹٲ� ���� �Ϸ� �ϴµ�, �ѱ��� �ڸ� �� ©���ų� �ϴ� �� ������
	// ��� ����, ã�ƺ��ϱ� for�� ������ if(char & 0x80) �� �ѱ� Ȯ���ؼ� ó���ϴ���.. ���ذ� �ȵ�. �ϴ� ����

	/*do
	{
		SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_ADDSTRING, 0,
			(LPARAM)TEXT(
				(message.substr(i * substrSize, substrSize)).c_str()));

		i++;
	} while (i < message.size() / substrSize);*/

}
