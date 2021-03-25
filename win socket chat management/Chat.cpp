#include "Chat.h"
#include "ChatLobby.h"
#include <string>
#include <atlbase.h>

using namespace std;

HWND chatDlgHandle;
extern vector<chattingRoomHwnd> chattingDlgVector;
extern vector<downLoadFileLine> downLoadFileLineVector;
extern HWND hChatLobbyDlg;

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
		case IDC_LIST_CHAT_LOG:
			switch (HIWORD(wParam))
			{
			case LBN_DBLCLK:
				// for 돌아서 값중에 인덱스 맞는게 있으면, 서버로 파일좀 보내줘 요청한다음에 파일 받기 실행,
				for (auto iterator = downLoadFileLineVector.begin(); iterator != downLoadFileLineVector.end();)
				{
					if (((*iterator).hwnd == hDlg) &&
						((*iterator).line == SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0)))
					{
						Json::Value value;
						value["kind"] = FileMessage;
						value["fileName"] = (*iterator).fileName;
						Client::GetInstance()->SendPacketToServer(value);
						// 해당 파일 이름 보내달라고 요청
						break;
					}
					else
					{
						iterator++;
					}
				}
				break;
			}
			break;
		case IDC_SEND_BTN:
			SendMessageToServer(hDlg);
			break;
		case IDC_FILE_BTN:
			GetCurrentDirectory(256, curDirectoryPath);         // GetOpenFileName 호출하면 기본 경로명이 바뀌기 때문에 기본 경로명 미리 저장

			ZeroMemory(&openFileName, sizeof(openFileName));    // 구조체를 0으로 셋업
			openFileName.lStructSize = sizeof(openFileName);
			openFileName.hwndOwner = hDlg;
			openFileName.lpstrTitle = "파일첨부";
			openFileName.lpstrFileTitle = strFileTitle;
			openFileName.lpstrFile = strFilePath;
			openFileName.nFilterIndex = 1;
			openFileName.lpstrFilter = NULL;
			openFileName.nMaxFile = MAX_PATH;
			openFileName.nMaxFileTitle = MAX_PATH;

			if (GetOpenFileName(&openFileName) != 0)    // 인덱스가 1부터 시작하기 때문에 지정
			{
				strcat(strFilePath, "\0");
				long fileSize;	// 파일 전체 사이즈
				FILE* fp;
				fopen_s(&fp, strFilePath, "rb");	// 파일 열고
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
			SetCurrentDirectory(curDirectoryPath);  // 변경된 경로를 원래 경로로 설정
			break;
		}
		break;
	case WM_CLOSE:
		for (auto iterator = chattingDlgVector.begin(); iterator != chattingDlgVector.end();)
		{
			if (hDlg == (*iterator).hwnd)
			{
				chattingDlgVector.erase(iterator);
				break;
			}
			else
				iterator++;
		}
		
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

	string message;
	switch (value["kind"].asInt())
	{
	case Message:
		message = value["name"].asString() + " : " + value["message"].asString();
		break;
	case FileMessage:
		message = value["message"].asString();

		// 리스트박스 파일을 보낸 메시지, (제일 마지막 메시지) 번호를 기억하고 있다가 나중에 사용
		downLoadFileLineVector.emplace_back(hDlg, (SendMessage(GetDlgItem(hDlg, IDC_LIST_CHAT_LOG), LB_GETCOUNT, 0, 0) + 1),
			value["fileName"].asString());
		break;
	}

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
