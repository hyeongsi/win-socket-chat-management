#pragma once
#include <string>
#include "Client.h"
#include "resource.h"

enum LoginFailed
{
	LoginSuccess,
	WringIdOrPassword,
	Cancel,
	Ban,
};

Json::Value LoginCheck(std::string id, std::string pw);
bool ConnectServer(std::string ip, std::string port);