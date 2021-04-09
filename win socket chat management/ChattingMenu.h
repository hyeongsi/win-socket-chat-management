#pragma once
#include <WinSock2.h>
#include "Client.h"

class ChattingMenu
{
	static ChattingMenu* instance;
	HWND chatDlgHandle;
	HINSTANCE g_hInst;

	ChattingMenu();
public:
	static ChattingMenu* GetInstance();
	static void ReleaseInstance();

	void SetInst(HINSTANCE g_hInst);

	static BOOL CALLBACK ChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static void GetFileDataMessage(HWND hDlg);
	void GetFileDataMessageMethod(HWND hDlg);
	static void SendMessageToServer(HWND hwnd);
	void SendMessageToServerMethod(HWND hwnd);
	static void SendFileDataButton(HWND hDlg);
	void SendFileDataBtnMethod(HWND hDlg);
	static void AddUserToChatButton(HWND hDlg);
	void AddUserToChatBtnMethod(HWND hDlg);
	static void EmoticonButton(HWND hDlg);
	void EmoticonBtnMethod(HWND hDlg);

	void SyncChatUI(HWND hDlg, Json::Value value);
};

