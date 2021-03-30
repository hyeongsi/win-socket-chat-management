#include "ChattingRoom.h"

constexpr const int PACKET_SIZE = 1024;

ChattingRoom::ChattingRoom(int chattingRoomNumber, string roomName)
{
	this->chattingRoomNumber = chattingRoomNumber;
	this->roomName = roomName;
}

int ChattingRoom::GetChattingRoomNumber()
{
	return chattingRoomNumber;
}

string ChattingRoom::GetChattingRoomName()
{
	return roomName;
}

void ChattingRoom::ConnectChattingRoom(SOCKET socket)
{
	for (auto iterator : userList)
	{
		if (socket == iterator)
			return;
	}

	userList.emplace_back(socket);
}

void ChattingRoom::SendChatting(Json::Value sendValue)
{
	for (auto iterator : userList)
	{
		SendJsonData(sendValue, iterator);
	}
}

bool ChattingRoom::SendJsonData(Json::Value value, SOCKET socket)
{
	string jsonString;
	char cBuffer[PACKET_SIZE] = {};

	Json::StyledWriter writer;
	jsonString = writer.write(value);

	memcpy(cBuffer, jsonString.c_str(), jsonString.size());

	if (send(socket, cBuffer, PACKET_SIZE, 0) == -1)
		return false;
	else
		return true;
}
