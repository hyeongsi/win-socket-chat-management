#pragma once
#include <string>
#include "Client.h"
#include "resource.h"

enum LoginFailed
{
	LoginSuccess = 0,
	WrongIdOrPassword = 1,
	Cancel = 2,
	Ban = 3,
};

Json::Value LoginCheck(std::string id, std::string pw);
bool ConnectServer(std::string ip, std::string port);