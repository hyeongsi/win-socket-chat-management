#pragma once
#include <string>
#include "Client.h"
#include "resource.h"

enum LoginFailed
{
	Success,
	NotFoundId,
	WrongPassword,
};

int LoginCheck();
bool ConnectServer(std::string ip, std::string port);