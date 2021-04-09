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

	string GetMyIP();	// 서버 컴퓨터 ip 추출
	string GetUserIdInUserList(); // 유저창의 id값 추출
	void MoveScrollbarToEnd(HWND hwnd);	// 스크롤바 최하단 이동
	void AdjustListboxHScroll(HWND hwnd);	// Hscroll 크기 자동 보정
	void DebugLogUpdate(int kind, string message);	// 로그 출력

	void InitDialogMethod(HWND hDlg);
	void CheckConnectUserBtnMethod();	// 현재 접속자 확인,출력
	void CheckUserIdListBtnMethod();	// 모든 사용자 확인,출력
	void BanBtnMethod();	// 사용자 밴
	void UnBanBtnMethod();	// 사용자 밴 취소
	void SaveServerLogBtnMethod(); // 로그 DB에 저장
};

