#pragma once
#include <WinSock2.h>
#include <list>
#include <string>

constexpr const int PORT = 4567;
constexpr const int PACKET_SIZE = 1024;

using namespace std;

typedef struct UserDatas
{
	SOCKET socket;
	string id;
	string name;
	UserDatas(SOCKET _socket, string _id, string _name)
		: socket(_socket), id(_id), name(_name) {};
}UserData;

class Server
{
private:
	static Server* instance;

	Server();
public:
	WSADATA wsaData;
	list<UserData> clientSocketList;

	static Server* GetInstance();
	static void ReleaseInstance();

	

};