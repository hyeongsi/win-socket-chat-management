#pragma once

#include "Login.h"

enum SignUpMessage
{
	ExsistsSameId = 1,
	ExsistsSameName,
};

constexpr const int ipEditboxCharSize = 30;

BOOL CALLBACK SignUpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
std::string GetDlgItemString(HWND hwnd);
void SignUpOkBtnMethod(HWND hDlg, WPARAM wParam);