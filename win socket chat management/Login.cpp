#include "Login.h"

using namespace std;

int LoginCheck()
{
	// id, pw ������ ���̽��� �� �� �α���
	return Success;
}

bool ConnectServer(std::string ip, std::string port)
{
	Client::GetInstance()->CloseSocket();	// ���� ���� ���� �ٽ� ���� ����
	
	try
	{
		if (!(Client::GetInstance()->ConnectInit(ip, stoi(port))))	// ���� ���� ��
			return false;
			//return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
	
	return true;
}