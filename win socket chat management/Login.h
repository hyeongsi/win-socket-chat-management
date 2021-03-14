#pragma once
#include <string>

enum LoginFailed
{
	Success,
	NotFoundId,
	WrongPassword,
};

int LoginCheck();
bool ConnectServer(std::string ip, std::string port);