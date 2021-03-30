#pragma once
#include <map>
#include "ChattingRoom.h"
#include "MembershipDB.h"

class ChattingRoomManager
{
private:
	const char* CHATTINGROOM_INFO_PATH = "chattingroomData\\chattingRoomData.csv";
	
	static ChattingRoomManager* instance;
	list<ChattingRoom*> chattingRoomList;

	ChattingRoomManager();
	~ChattingRoomManager();
public:
	const char* CHATTINGROOM_USER_INFO_PATH = "chattingroomData\\chattingRoomUserData.csv";

	static ChattingRoomManager* GetInstance();
	static void ReleaseInstance();

	list<ChattingRoom*> GetChattingRoomList();
};

