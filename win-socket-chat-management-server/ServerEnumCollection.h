#pragma once

enum SignUpMessage
{
	Success = 0,
	ExsistsSameId = 1,
	ExsistsSameName = 2,
};

enum MessageKind
{
	SignUp = 0,
	Login = 1,
	ChattingRoomInit = 2,
	AddChattingRoom = 3,
	AddChattingRoomUser = 4,
	GetChattringRoomName = 5,
	GetFriendData = 6,
	Message = 7,
	GetFileRequest = 8,
	SetFileRequest = 9,
	Emoticon = 10,
	AddFriend = 11,
};

enum EmoticonKind
{
	Happy = 0,
	Sad = 1,
	Surprised = 2,
};