#include "Login.h"

using namespace std;

Json::Value LoginCheck(string id, string pw)
{
	Client::GetInstance()->SendLoginSignToServer(id, pw);

	Json::Value recvValue = Client::GetInstance()->RecvPacketToServer();
	if (recvValue == NULL)
		return Cancel;

	return recvValue;
}

bool ConnectServer(string ip, string port)
{
	Client::GetInstance()->CloseSocket();	// ���� ���� ���� �ٽ� ���� ����
	
	try
	{
		if (!(Client::GetInstance()->ConnectInit(ip, stoi(port))))	// ���� ���� ��
			return false;
	}
	catch (const std::exception& a)
	{
		printf("%s", a.what());
		return false;
	}
	
	return true;
}