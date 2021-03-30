#pragma once
#include <WinSock2.h>
#include <list>
#include <string>
#include "jsoncppkor\include\json\json.h"

using namespace std;

class ChattingRoom
{
private:
	int chattingRoomNumber = 0;
	string roomName;
	list<SOCKET> userList;

	bool SendJsonData(Json::Value value, SOCKET socket);
public:
	ChattingRoom(int chattingRoomNumber, string roomName);

	int GetChattingRoomNumber();
	string GetChattingRoomName();
	void ConnectChattingRoom(SOCKET socket);
	void SendChatting(Json::Value sendValue);

};

