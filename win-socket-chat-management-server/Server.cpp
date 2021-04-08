#include "Server.h"
Server* Server::instance = nullptr;
Server::Server() { }

Server* Server::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new Server();
    }

    return instance;
}

void Server::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}