#include "Login.h"

using namespace std;

int LoginCheck()
{
	// id, pw 데이터 베이스와 비교 후 로그인
	return Success;
}

bool ConnectServer(std::string ip, std::string port)
{
	Client::GetInstance()->CloseSocket();	// 기존 연결 끊고 다시 연결 진행
	
	try
	{
		if (!(Client::GetInstance()->ConnectInit(ip, stoi(port))))	// 연결 실패 시
			return false;
			//return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
	
	return true;
}