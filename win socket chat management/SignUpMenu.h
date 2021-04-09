#pragma once
#include <WinSock2.h>
#include <string>

using namespace std;

enum SignUpMessage
{
	ExsistsSameId = 1,
	ExsistsSameName = 2,
};

class SignUpMenu
{
private:
	static SignUpMenu* instance;

	SignUpMenu();
public:
	static SignUpMenu* GetInstance();
	static void ReleaseInstance();

	static BOOL CALLBACK SignUpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
	
	static void SignUpOkButton(HWND hDlg, WPARAM wParam);
	void SignUpOkBtnMethod(HWND hDlg, WPARAM wParam);

	string GetDlgItemString(HWND hwnd);
};

