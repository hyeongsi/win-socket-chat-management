#pragma once

#include "Login.h"

enum SignUpMessage
{
	ExsistsSameId,
	ExsistsSameName,
};

constexpr const int ipEditboxCharSize = 30;

BOOL CALLBACK SignUpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);
std::string GetDlgItemString(HWND hwnd);