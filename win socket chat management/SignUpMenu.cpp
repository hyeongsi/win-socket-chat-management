﻿#include "SignUpMenu.h"
#include "resource.h"
#include "Client.h"
#include "LoginMenu.h"

SignUpMenu* SignUpMenu::instance = nullptr;
SignUpMenu::SignUpMenu() {};

SignUpMenu* SignUpMenu::GetInstance()
{
	if (nullptr == instance)
	{
		instance = new SignUpMenu();
	}

	return instance;
}

void SignUpMenu::ReleaseInstance()
{
	delete instance;
	instance = nullptr;
}

BOOL SignUpMenu::SignUpDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP, 120, 120, 0, 0, SWP_NOSIZE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SIGNUP_OK_BTN:
			SignUpOkButton(hDlg, wParam);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

void SignUpMenu::SignUpOkButton(HWND hDlg, WPARAM wParam)
{
	GetInstance()->SignUpOkBtnMethod(hDlg, wParam);
}

void SignUpMenu::SignUpOkBtnMethod(HWND hDlg, WPARAM wParam)
{
	if (
		!(LoginMenu::GetInstance()->ConnectServer(GetDlgItemString(GetDlgItem(hDlg, IDC_EDIT_SIGNUP_IP)),
			GetDlgItemString(
				GetDlgItem(hDlg, IDC_EDIT_SIGNUP_PORT)
			))))
	{
		MessageBox(hDlg, "서버 연결 실패", "로그인 오류", NULL);
		return;
	}

	Json::Value root;
	root["kind"] = SignUp;
	root["id"] = GetDlgItemString(GetDlgItem(hDlg, IDC_EDIT_SIGNUP_ID));
	root["pw"] = GetDlgItemString(GetDlgItem(hDlg, IDC_EDIT_SIGNUP_PW));
	root["pw2"] = GetDlgItemString(GetDlgItem(hDlg, IDC_EDIT_SIGNUP_PW2));
	if (root["pw"].asString() != root["pw2"].asString())
	{
		MessageBox(hDlg, "확인 비밀번호가 다릅니다.", "error", 0);
		return;
	}
	root["name"] = GetDlgItemString(GetDlgItem(hDlg, IDC_EDIT_SIGNUP_NICKNAME));

	Client::GetInstance()->SendPacketToServer(root);
	Json::Value resultRoot;
	resultRoot = Client::GetInstance()->RecvPacketToServer();
	if (resultRoot == NULL)
	{
		MessageBox(hDlg, "서버연결 실패", "fail", 0);
		Client::GetInstance()->CloseSocket();
		return;
	}

	if (resultRoot["result"].asBool())
	{
		Client::GetInstance()->CloseSocket();
		MessageBox(hDlg, "회원가입 성공", "success", 0);
		EndDialog(hDlg, wParam);
	}
	else
	{
		switch (resultRoot["value"].asInt())
		{
		case ExsistsSameId:
			MessageBox(hDlg, "동일한 ID 존재합니다.\n 회원가입 실패", "fail", 0);
			break;
		case ExsistsSameName:
			MessageBox(hDlg, "동일한 Name 존재합니다.\n 회원가입 실패", "fail", 0);
			break;
		default:
			MessageBox(hDlg, "회원가입 실패", "fail", 0);
			break;
		}

		Client::GetInstance()->CloseSocket();
	}
}

string SignUpMenu::GetDlgItemString(HWND hwnd)
{
	char temptStr[ipEditboxCharSize];
	GetWindowText(hwnd, temptStr, ipEditboxCharSize);
	return string(temptStr);
}
