#pragma once
#include <WinSock2.h>
#include <string>
#include <time.h>
#include "resource.h"
#include "MembershipDB.h"
#include "ServerUIEnumCollection.h"

using namespace std;

class ServerUI
{
private:
	const string SAVE_LOG_PATH = "saveLog\\serverLogData.csv";

	static ServerUI* instance;
	time_t curTime = time(NULL);
	struct tm localTime = *localtime(&curTime);

	ServerUI();
public:
	HWND g_hDlg;

	static ServerUI* GetInstance();
	static void ReleaseInstance();

	string GetMyIP();	// ���� ��ǻ�� ip ����
	string GetUserIdInUserList(); // ����â�� id�� ����
	void MoveScrollbarToEnd(HWND hwnd);	// ��ũ�ѹ� ���ϴ� �̵�
	void AdjustListboxHScroll(HWND hwnd);	// Hscroll ũ�� �ڵ� ����
	void DebugLogUpdate(int kind, string message);	// �α� ���

	void InitDialogMethod(HWND hDlg);
	void CheckConnectUserBtnMethod();	// ���� ������ Ȯ��,���
	void CheckUserIdListBtnMethod();	// ��� ����� Ȯ��,���
	void BanBtnMethod();	// ����� ��
	void UnBanBtnMethod();	// ����� �� ���
	void SaveServerLogBtnMethod(); // �α� DB�� ����
};

