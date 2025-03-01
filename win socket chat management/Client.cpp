﻿#include "Client.h"
#include <shlwapi.h> 
#include <ShlObj_core.h>
#pragma comment(lib, "shlwapi")

using namespace std;

Client* Client::instance = nullptr;
Client::Client(){ }

Client* Client::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new Client();
    }

    return instance;
}

void Client::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

bool Client::ConnectInit(string ip, int port)
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) != 0)
		return false;

	return true;
}

void Client::CloseSocket()
{
	if (clientSocket != NULL)
		return;

	closesocket(clientSocket);
	WSACleanup();
}

void Client::SendLoginSignToServer(string id, string pw)
{
	Json::Value root;
	root["id"] = id;
	root["pw"] = pw;
	root["kind"] = Login;

	SendPacketToServer(root);
}

bool Client::SendMessageToServer(std::string msg, const int roomNumber)
{
	if (!msg.size())
		return false;

	Json::Value root;
	root["kind"] = Message;
	root["message"] = msg.c_str();
	root["roomNumber"] = roomNumber;

	return SendPacketToServer(root);
}

bool Client::SendPacketToServer(Json::Value root)
{
	char cBuffer[PACKET_SIZE];
	string str;

	Json::StyledWriter writer;
	str = writer.write(root);

	memcpy(cBuffer, str.c_str(), str.size());

	if (send(clientSocket, cBuffer, PACKET_SIZE, 0) == -1)
	{
		MessageBox(g_hDlg, "send error", "error", NULL);
		// 서버 재접속 코드 작성
		return false;
	}

	return true;
}

bool Client::SendFileDataToServer(FILE* fp, int fileSize)
{
	int sendBytes;	// 읽어온 파일 사이즈 저장할 변수
	char cBuffer[PACKET_SIZE];
	
	snprintf(cBuffer, sizeof(cBuffer), "%d", fileSize);

	while (1)
	{
		sendBytes = fread(cBuffer, sizeof(char), PACKET_SIZE, fp);
		if (send(clientSocket, cBuffer, sendBytes, 0) == -1)
		{
			MessageBox(g_hDlg, "send error", "error", NULL);
			// 서버 재접속 코드 작성
			return false;
		}

		if (feof(fp))
			break;
	}

	return true;
}

bool Client::RecvFileData(Json::Value value)
{
	char cBuffer[PACKET_SIZE];
	int totalRecvFileCount, currentRecvFileCount = 0;
	int fileSize;
	int readByteSize;
	string recvFilePath;

	char cpath[MAX_PATH] = ("");
	LPITEMIDLIST pDirList;
	BROWSEINFO browseInfo;
	browseInfo.hwndOwner = NULL;
	browseInfo.pidlRoot = NULL;
	browseInfo.lpszTitle = ("폴더를 선택해 주세요");
	browseInfo.pszDisplayName = cpath;
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	browseInfo.lpfn = NULL;
	browseInfo.lParam = 0;
	pDirList = SHBrowseForFolder(&browseInfo);
	if (pDirList == NULL) return false;
	SHGetPathFromIDList(pDirList, cpath);
	char* return_path = cpath;

	currentRecvFileCount = 0;
	fileSize = value["fileSize"].asInt();
	totalRecvFileCount = fileSize / PACKET_SIZE + 1;

	recvFilePath = value["fileName"].asString();
	int fileNumber = 0;

	while (isExistFile(string(return_path) + "\\" + value["fileName"].asString(), fileNumber))
	{
		fileNumber++;
	}

	recvFilePath = string(return_path) + "\\" + value["fileName"].asString();

	for (int i = recvFilePath.size() - 1; i >= 0; i--)
	{
		if (!fileNumber)
			break;

		if (recvFilePath[i] != '.')
			continue;

		recvFilePath.insert(i, "(" + to_string(fileNumber) + ")");
		break;
	}

	FILE* fp;
	fopen_s(&fp, (recvFilePath).c_str(), "wb");
	if (fp != NULL)
	{
		while (currentRecvFileCount != totalRecvFileCount)
		{
			readByteSize = recv(clientSocket, cBuffer, PACKET_SIZE, 0);
			currentRecvFileCount++;
			fwrite(cBuffer, sizeof(char), readByteSize, fp);
		}

		fclose(fp);
		return true;
	}

	return false;
}

Json::Value Client::RecvPacketToServer()
{
	char cBuffer[PACKET_SIZE] = {};
	int recvResult;

	if ((recvResult = recv(clientSocket, cBuffer, PACKET_SIZE, 0)) != -1)
	{
		Json::Reader reader;
		Json::Value recvValue;
		reader.parse(cBuffer, recvValue);

		return recvValue;
	}

	return NULL;
}

bool Client::isExistFile(string filePath, int index)
{
	if (!index)
		return PathFileExists(filePath.c_str());

	for (int i = filePath.size()-1; i >= 0; i--)
	{
		if (filePath[i] == '.')
		{
			filePath.insert(i, "(" + to_string(index) + ")");
			break;
		}
	}


	return PathFileExists(filePath.c_str());
}


