#pragma once
#include <WinSock2.h>
#include "Client.h"

class InputMenu
{
private:
	static InputMenu* instance;

	InputMenu();
public:
	char inputFriendIdInChat[PACKET_SIZE];
	int selectEmoticon;

	static InputMenu* GetInstance();
	static void ReleaseInstance();

	static BOOL CALLBACK InputIDDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK InputRoomNameDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK AddFriendInChatDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK InputEmoticonDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

	static void SetInputFriendIdInChat(const char* str);
	static void SetSelectEmoticon(int selectEmoticon);
};

