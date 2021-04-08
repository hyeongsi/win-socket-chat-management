#include "server-main.h"
#pragma comment(lib,"ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	g_hInst = hInstance;
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), HWND_DESKTOP, MainDlgProc);

	ServerUI::ReleaseInstance();
	PostQuitMessage(0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		ServerUI::GetInstance()->InitDialogMethod(hDlg);	// ip, port, hDlg, UI 생성 위치 설정
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONNECT_USER_CHECK_BTN:	// 접속사 수 확인
			ServerUI::GetInstance()->CheckConnectUserBtnMethod();
			break;
		case ID_USER_CHECK_BTN:			// 모든 사용자 확인
			ServerUI::GetInstance()->CheckUserIdListBtnMethod();
			break;
		case ID_BAN_BUTTON:	// 계정 정지
			ServerUI::GetInstance()->BanBtnMethod();
			break;
		case ID_UNBAN_BUTTON:
			ServerUI::GetInstance()->UnBanBtnMethod();
			break;
		case ID_SERVER_SAVE_LOG_BTN:
			ServerUI::GetInstance()->SaveServerLogBtnMethod();
			break;
		case ID_START_SERVER_BTN:
			AcceptThreadHandle = (HANDLE)_beginthreadex(NULL, 0, StartServer, NULL, 0, NULL);
			break;
		case ID_STOP_SERVER_BTN:
			StopServer();
			break;
		}
		break;
	case WM_CLOSE:
		closesocket(serverSocket);
		WSACleanup();
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	return FALSE;
}

bool InitServer()
{
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 == bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)))
	{
		MessageBox(g_hDlg, "bind Error", "error", NULL);
		return false;
	}

	if (-1 == listen(serverSocket, SOMAXCONN))
	{
		MessageBox(g_hDlg, "listen Error", "error", NULL);
		return false;
	}

	return true;
}

unsigned WINAPI RecvThread(void* arg)
{
	SOCKET clientSocket = *(SOCKET*)arg;
	char cBuffer[PACKET_SIZE] = {};
	string userId, userName;

	while ((recv(clientSocket, cBuffer, PACKET_SIZE, 0)) != -1)
	{
		if (!isOpenServer)
		{
			closesocket(clientSocket);
			return 0;
		}

		Json::Reader reader;
		Json::Value recvValue, sendValue;
		reader.parse(cBuffer, recvValue);

		string test;

		switch (recvValue["kind"].asInt())
		{
		case SignUp:
			SignUpMessageMethod(recvValue, &clientSocket);
			break;
		case Login:
			LoginMessageMethod(recvValue, &userId, &userName, &clientSocket);
			break;
		case ChattingRoomInit:
			ChattingRoomInitMethod(sendValue, &userId, &userName, &clientSocket);
			break;
		case AddChattingRoom:
			AddChattingRoomMethod(recvValue, sendValue, &userId ,&userName, &clientSocket);
			break;
		case AddChattingRoomUser:
			AddChattingRoomUserMethod(recvValue, sendValue, &userId, &clientSocket);
			break;
		case GetChattringRoomName:
			GetChattingRoomNameMethod(recvValue, sendValue, &clientSocket);
			break;
		case GetFriendData:
			GetFriendDataMethod(sendValue, &userId, &clientSocket);
			break;
		case Message:
			JsonMessageMethod(recvValue, &userId, &userName);
			break;
		case SetFileRequest:
			SetFileRequestMessageMethod(recvValue, &userName, &clientSocket);
			break;
		case AddFriend:
			AddFriendMessageMethod(recvValue, &userId, &clientSocket);
			break;
		case GetFileRequest:
			GetFileRequestMessageMethod(recvValue, &userName, &clientSocket);
			break;
		case Emoticon:
			EmoticonMessageMethod(recvValue, sendValue, &userName, &clientSocket);
			break;
		default:
			return 0;
		}
	}

	ExitClient(&clientSocket);
	return 0;
}

unsigned WINAPI StartServer(void* arg)
{
	if (isOpenServer)
		return 0;

	if (!InitServer())
		return 0;

	DebugLogUpdate(logBox, "서버가 시작되었습니다.");
	isOpenServer = true;

	int clientAddressSize = sizeof(clientAddress);
	while (isOpenServer)
	{
		clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &clientAddressSize);
		if (SOCKET_ERROR != clientSocket)
		{
			_beginthreadex(NULL, 0, RecvThread, &clientSocket, 0, NULL);
		}
	}

	return 0;
}

void StopServer()
{
	if (!isOpenServer)
		return;

	int count = SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCOUNT, 0, 0);
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_DELETESTRING, 0, 0);
		}
	}

	closesocket(serverSocket);
	AcceptThreadHandle = nullptr;

	isOpenServer = false;
	DebugLogUpdate(logBox, "서버가 종료되었습니다.");
}

void SignUpMessageMethod(Json::Value recvValue, SOCKET* clientSocket)
{
	Json::Value sendValue;

	DebugLogUpdate(logBox, recvValue["id"].asString() + ", " +
		recvValue["pw"].asString() + ", " + recvValue["name"].asString() + " 회원가입 요청");

	sendValue["value"] = CheckSignUpData(recvValue["id"].asString(),
		recvValue["pw"].asString(), recvValue["name"].asString());

	if (Success == sendValue["value"].asInt())
	{
		vector<string> writeData;
		writeData.emplace_back(recvValue["id"].asString());
		writeData.emplace_back(recvValue["pw"].asString());
		writeData.emplace_back(recvValue["name"].asString());

		MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, writeData);
		writeData.clear();
		writeData.emplace_back(recvValue["id"].asString());
		MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->FRIEND_LIST_PATH, writeData);
		writeData.clear();
		writeData.emplace_back(recvValue["id"].asString());
		MembershipDB::GetInstance()->WriteDataToCsv(ChattingRoomManager::GetInstance()->CHATTINGROOM_USER_INFO_PATH, writeData);
		DebugLogUpdate(logBox, "회원가입 성공");
		// db에 데이터 저장;
		SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), BM_CLICK, 0, 0);
		sendValue["result"] = true;
	}
	else
	{
		DebugLogUpdate(logBox, "회원가입 실패");
		sendValue["result"] = false;
	}

	if (!SendJsonData(sendValue, *clientSocket))
		return;
}

void LoginMessageMethod(Json::Value recvValue , string* userId, string* userName, SOCKET* clientSocket)
{
	Json::Value sendValue;

	sendValue["result"] = MembershipDB::GetInstance()->
		LoginCheck(recvValue["id"].asString(),
			recvValue["pw"].asString(), &sendValue);

	if (!SendJsonData(sendValue, *clientSocket))
		return;

	if (LoginSuccess == sendValue["result"].asInt())
	{
		*userId = recvValue["id"].asString();
		*userName = MembershipDB::GetInstance()->FindName(recvValue["id"].asString());

		/*clientSocketListMutex.lock();
		clientSocketList.emplace_back(UserData(
			*clientSocket,
			*userId,
			*userName));
		clientSocketListMutex.unlock();*/

		ServerUI::GetInstance()->CheckUserIdListBtnMethod();
		DebugLogUpdate(logBox, *userId + ", " + *userName + " 유저 접속");
	}
}

void JsonMessageMethod(Json::Value recvValue, string* userId, string* userName)
{
	Json::Value sendValue;
	
	DebugLogUpdate(logBox, *userId + " / " + *userName + " / " + to_string(recvValue["roomNumber"].asInt()) +
		" / message : " + recvValue["message"].asString());

	sendValue["kind"] = Message;
	sendValue["name"] = *userName;
	sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
	sendValue["message"] = recvValue["message"].asString();
	
	if (0 == recvValue["roomNumber"].asInt())
	{
		/*clientSocketListMutex.lock();
		for (const auto& iterator : clientSocketList)
		{
			SendJsonData(sendValue, iterator.socket);
		}
		clientSocketListMutex.unlock();*/
		return;
	}

	for (const auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		if (iterator->GetChattingRoomNumber() != recvValue["roomNumber"].asInt())
			continue;

		iterator->SendChatting(sendValue);
		break;
	}
}

void SetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket)
{
	Json::Value sendValue;
	int totalRecvFileCount, currentRecvFileCount = 0;
	int fileSize;
	int readByteSize;
	char cBuffer[PACKET_SIZE];

	currentRecvFileCount = 0;
	fileSize = recvValue["fileSize"].asInt();
	totalRecvFileCount = fileSize / PACKET_SIZE + 1;

	FILE* sfp;
	fopen_s(&sfp, ("downloadFiles\\" + recvValue["fileName"].asString()).c_str(), "wb");
	if (sfp != NULL)
	{
		while (currentRecvFileCount != totalRecvFileCount)
		{
			readByteSize = recv(*clientSocket, cBuffer, PACKET_SIZE, 0);
			currentRecvFileCount++;
			fwrite(cBuffer, sizeof(char), readByteSize, sfp);
		}

		fclose(sfp);
	}

	DebugLogUpdate(logBox, *userName + " " +
		recvValue["fileName"].asString() + "수신완료");

	sendValue["kind"] = GetFileRequest;
	sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
	sendValue["message"] = *userName + "님이 " +
		recvValue["fileName"].asString() + "파일을 보냈습니다.";
	sendValue["fileName"] = recvValue["fileName"].asString();
	sendValue["roomNumber"] = recvValue["roomNumber"].asInt();

	if (0 == recvValue["roomNumber"].asInt())
	{
		clientSocketListMutex.lock();
		/*for (const auto& iterator : clientSocketList)
		{
			SendJsonData(sendValue, iterator.socket);
		}
		clientSocketListMutex.unlock();*/
		return;
	}

	for (const auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		if (iterator->GetChattingRoomNumber() != recvValue["roomNumber"].asInt())
			continue;

		iterator->SendChatting(sendValue);
		break;
	}
}

void AddFriendMessageMethod(Json::Value recvValue, string* userId, SOCKET* clientSocket)
{
	Json::Value sendValue;
	int findReturnValue;

	sendValue["kind"] = AddFriend;
	if (recvValue["friendId"].asString() == *userId)
	{
		sendValue["result"] = false;
		sendValue["message"] = "자기 자신은 친구로 등록할 수 없습니다.";
		SendJsonData(sendValue, *clientSocket);
		return;
	}

	findReturnValue = MembershipDB::GetInstance()->ExistValue(
		MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, ID, recvValue["friendId"].asString());

	if (findReturnValue < 0)
	{
		sendValue["result"] = false;
		sendValue["message"] = "해당 id는 없는 id 입니다.";
		SendJsonData(sendValue, *clientSocket);
		return;
	}

	int count = 0;
	bool isExistValue = false;
	vector<string> row;
	for (const auto iterator : MembershipDB::GetInstance()->GetColumn(MembershipDB::GetInstance()->FRIEND_LIST_PATH))
	{
		if (0 == count)
		{
			FILE* fp = fopen(MembershipDB::GetInstance()->FRIEND_LIST_PATH, "w");
			fclose(fp);
			count++;
		}

		vector<string> fixedRow;
		vector<string> row = MembershipDB::GetInstance()->Split(iterator, ',');

		for (int i = 0; i < row.size(); i++)
		{
			if (row[i] != "")
				fixedRow.emplace_back(row[i]);
		}

		if (fixedRow[0] == *userId)
		{
			for (int i = 1; i < fixedRow.size(); i++)
			{
				if (fixedRow[i] == recvValue["friendId"].asString())
				{
					isExistValue = true;
					break;
				}
			}

			if(!isExistValue)
				fixedRow.emplace_back(recvValue["friendId"].asString());
		}


		MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->FRIEND_LIST_PATH,
			fixedRow);
	}

	if (isExistValue)
	{
		sendValue["result"] = false;
		sendValue["message"] = "이미 등록되어 있는 친구입니다.";
	}
	else
	{
		sendValue["result"] = true;
		sendValue["friendId"] = recvValue["friendId"].asString();
	}
	
	SendJsonData(sendValue, *clientSocket);
}

void GetFileRequestMessageMethod(Json::Value recvValue, string* userName, SOCKET* clientSocket)
{
	Json::Value sendValue;
	long fileSize;	// 파일 전체 사이즈
	FILE* fp;

	DebugLogUpdate(logBox, *userName + "파일전송요청 수신");
	fopen_s(&fp, ("downloadFiles\\" + recvValue["fileName"].asString()).c_str(), "rb");	// 파일 열고
	if (fp != NULL)
	{	// 파일을 보냄
		DebugLogUpdate(logBox, *userName + recvValue["fileName"].asString() + " 파일전송시작");
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		sendValue["kind"] = SetFileRequest;
		sendValue["fileSize"] = (int)fileSize;
		sendValue["fileName"] = recvValue["fileName"].asString();
		sendValue["roomNumber"] = recvValue["roomNumber"].asInt();

		SendJsonData(sendValue, *clientSocket);
		SendFileDataToServer(fp, fileSize, *clientSocket);
		fclose(fp);
		DebugLogUpdate(logBox, *userName + recvValue["fileName"].asString() + " 파일전송성공");
	}
	else
		DebugLogUpdate(logBox, *userName + recvValue["fileName"].asString() + " 파일전송실패");
}

void ChattingRoomInitMethod(Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket)
{
	list<ChattingRoom*> chattingRoomList;
	int count = 0;
	string chattingRoomNumberStr;
	vector<string> copyRoomNumber;

	for (auto& loadTextIterator : MembershipDB::GetInstance()->GetColumn(
		ChattingRoomManager::GetInstance()->
		CHATTINGROOM_USER_INFO_PATH))
	{
		if (!count)
		{
			count++;
			continue;   // 첫 줄 무시
		}
			
		vector<string> row = MembershipDB::GetInstance()->Split(loadTextIterator, ',');
		if (row[0] == *userId)
		{
			copyRoomNumber = row;
			break;
		}

		row.clear();
		count++;
	}

	if (copyRoomNumber.size() <= 0)
	{
		sendValue["name"] = *userName;
		sendValue["id"] = *userId;
		sendValue["roomNumberStr"] = "";
		SendJsonData(sendValue, *clientSocket);
		return;
	}
		
	// 채팅방 연결 처리
	for (auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		for (auto i = 1; i != copyRoomNumber.size(); i++)
		{
			try
			{
				if (iterator->GetChattingRoomNumber() == stoi(copyRoomNumber[i]))
				{
					iterator->ConnectChattingRoom(*clientSocket);
				}
			}
			catch (const std::exception&)
			{

			}
		}
	}

	for (auto i = 1; i != copyRoomNumber.size(); i++)
	{
		chattingRoomNumberStr += copyRoomNumber[i];

		if (i < copyRoomNumber.size() - 1)
			chattingRoomNumberStr += ",";
	}

	sendValue["name"] = *userName;
	sendValue["id"] = *userId;
	sendValue["roomNumberStr"] = chattingRoomNumberStr;
	SendJsonData(sendValue, *clientSocket);
	return;
}

void GetChattingRoomNameMethod(Json::Value recvValue, Json::Value sendValue, SOCKET* clientSocket)
{
	for (auto iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		if (iterator->GetChattingRoomNumber() == stoi(recvValue["roomNumber"].asString()))
		{
			sendValue["roomName"] = iterator->GetChattingRoomName();
			SendJsonData(sendValue, *clientSocket);
			return;
		}
	}
}

void AddChattingRoomMethod(Json::Value recvValue, Json::Value sendValue, string* userId, string* userName, SOCKET* clientSocket)
{
	DebugLogUpdate(logBox, (*userName + "의 " + recvValue["roomName"].asString() + " 채팅방 생성 요청"));
	sendValue["kind"] = AddChattingRoom;
	clientSocketListMutex.lock();
	sendValue["result"] = ChattingRoomManager::GetInstance()->AddChattingRoom(recvValue["roomName"].asString(), *userId);
	sendValue["roomName"] = recvValue["roomName"].asString();
	sendValue["roomNumber"] = ChattingRoomManager::GetInstance()->GetChattingRoomList().back()->GetChattingRoomNumber();
	clientSocketListMutex.unlock();
	SendJsonData(sendValue, *clientSocket);
	DebugLogUpdate(logBox, (*userName + "의 " + recvValue["roomName"].asString() + " 채팅방 생성"));
}

void AddChattingRoomUserMethod(Json::Value recvValue, Json::Value sendValue, string* userId, SOCKET* clientSocket)
{
	int count = 0;

	vector<string> row;
	for (const auto iterator : MembershipDB::GetInstance()->GetColumn(ChattingRoomManager::GetInstance()->CHATTINGROOM_USER_INFO_PATH))
	{
		if (0 == count)
		{
			FILE* fp = fopen(ChattingRoomManager::GetInstance()->CHATTINGROOM_USER_INFO_PATH, "w");
			fclose(fp);
			count++;
		}

		vector<string> row = MembershipDB::GetInstance()->Split(iterator, ',');
		vector<string> fixedRow;

		for (int i = 0; i < row.size(); i++)
		{
			if (row[i] != "")
				fixedRow.emplace_back(row[i]);
		}

		if (fixedRow[0] == recvValue["addUserId"].asString())
		{
			fixedRow.emplace_back(recvValue["roomNumber"].asString());
		}

		MembershipDB::GetInstance()->WriteDataToCsv(ChattingRoomManager::GetInstance()->CHATTINGROOM_USER_INFO_PATH,
			fixedRow);
	}

	if (recvValue["addUserId"].asString() == *userId)	// 본인 추가하기
	{
		for (const auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
		{
			if (iterator->GetChattingRoomName() == recvValue["roomName"].asString())
			{
				iterator->ConnectChattingRoom(*clientSocket);
				DebugLogUpdate(logBox, string(recvValue["addUserId"].asString() + "유저 추가 성공"));
				return;
			}	
		}
		return;
	}
		
	// 채팅방에 유저 추가하기
	clientSocketListMutex.lock();
	//for (const auto& iterator : clientSocketList)
	//{
	//	/*if (iterator.id == recvValue["addUserId"].asString())
	//	{
	//		for (const auto& roomListIterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	//		{
	//			if (roomListIterator->GetChattingRoomName() == recvValue["roomName"].asString())
	//			{
	//				roomListIterator->ConnectChattingRoom(iterator.socket);
	//				sendValue["kind"] = AddChattingRoomUser;
	//				DebugLogUpdate(logBox, string(recvValue["addUserId"].asString() + "유저 추가 성공"));
	//				sendValue["roomName"] = recvValue["roomName"].asString();
	//				sendValue["roomNumber"] = stoi(recvValue["roomNumber"].asString());
	//				SendJsonData(sendValue, iterator.socket);
	//				clientSocketListMutex.unlock();
	//				return;
	//			}
	//		}
	//	}*/
	//}

	clientSocketListMutex.unlock();
	DebugLogUpdate(logBox, string(recvValue["addUserId"].asString() + "유저 추가 성공"));
}

void GetFriendDataMethod(Json::Value sendValue, string* userId, SOCKET* clientSocket)
{
	string friendString = "";

	list<string> friendList =
		MembershipDB::GetInstance()->GetColumn(MembershipDB::GetInstance()->FRIEND_LIST_PATH);

	for (auto loadTextIterator = friendList.begin(); loadTextIterator != friendList.end(); )
	{
		if (loadTextIterator == friendList.begin())
		{
			loadTextIterator++;
			continue;   // 첫줄 제외
		}

		vector<string> row = MembershipDB::GetInstance()->Split(*loadTextIterator, ',');

		if (row[0] == *userId)
		{
			for(int i = 1; i < row.size(); i++)
			{
				friendString += row[i];

				if (i < row.size() - 1)
					friendString += ",";
			}

			sendValue["kind"] = GetFriendData;
			sendValue["friend"] = friendString;

			SendJsonData(sendValue, *clientSocket);
			break;
		}

		row.clear();
		loadTextIterator++;
	}
}

void EmoticonMessageMethod(Json::Value recvValue, Json::Value sendValue, string* userName, SOCKET* clientSocket)
{
	sendValue["kind"] = Message;
	sendValue["roomNumber"] = recvValue["roomNumber"].asInt();
	sendValue["name"] = *userName;

	switch (recvValue["emoticon"].asInt())
	{
	case Happy:
		sendValue["message"] = "    :-)";
		break;
	case Sad:
		sendValue["message"] = "    :-(";
		break;
	case Surprised:
		sendValue["message"] = "    :-O";
		break;
	default:
		break;
	}

	if (0 == recvValue["roomNumber"].asInt())
	{
		clientSocketListMutex.lock();
		/*for (const auto& iterator : clientSocketList)
		{
			SendJsonData(sendValue, iterator.socket);
		}*/
		clientSocketListMutex.unlock();
		return;
	}

	for (const auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		if (iterator->GetChattingRoomNumber() != recvValue["roomNumber"].asInt())
			continue;

		iterator->SendChatting(sendValue);
		break;
	}
}

void ExitClient(SOCKET* clientSocket)
{
	int count = 0;

	clientSocketListMutex.lock();
	//for (auto iterator = clientSocketList.begin(); iterator != clientSocketList.end();)
	//{
	//	/*if ((*iterator).socket == *clientSocket)
	//	{
	//		DebugLogUpdate(logBox, (*iterator).id + "유저 로그아웃");
	//		clientSocketList.erase(iterator);
	//		SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_DELETESTRING, count, 0);
	//		break;
	//	}
	//	else
	//	{
	//		count++;
	//		iterator++;
	//	}*/
	//}
	clientSocketListMutex.unlock();

	closesocket(*clientSocket);
}

int CheckSignUpData(string id, string pw, string name)
{
	switch (MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, ID, id))
	{
	case ID:
		return ExsistsSameId;
	case -2:
		return -2;
	}

	switch (MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, NAME, name))
	{
	case NAME:
		return ExsistsSameName;
	case -2:
		return -2;
	}

	return Success;
}

string JsonToString(Json::Value value)
{
	string str;
	Json::StyledWriter writer;
	str = writer.write(value);

	return str;
}

bool SendJsonData(Json::Value value, SOCKET socket)
{
	string jsonString;
	char cBuffer[PACKET_SIZE] = {};
	jsonString = JsonToString(value);
	memcpy(cBuffer, jsonString.c_str(), jsonString.size());

	if (send(socket, cBuffer, PACKET_SIZE, 0) == -1)
		return false;
	else
		return true;
}

bool SendFileDataToServer(FILE* fp, int fileSize, SOCKET socket)
{
	int sendBytes;	// 읽어온 파일 사이즈 저장할 변수
	char cBuffer[PACKET_SIZE];

	snprintf(cBuffer, sizeof(cBuffer), "%d", fileSize);

	while (1)
	{
		sendBytes = fread(cBuffer, sizeof(char), PACKET_SIZE, fp);
		if (send(socket, cBuffer, sendBytes, 0) == -1)
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

void DebugLogUpdate(int kind, string message)
{
	HWND listBox;
	string logMessage;

	switch (kind)
	{
	case userBox:
		listBox = GetDlgItem(g_hDlg, IDC_USERS_LIST);
		logMessage = message;
		break;
	case logBox:
		listBox = GetDlgItem(g_hDlg, IDC_LOG_LIST);

		logMessage = to_string(1900 + localTime.tm_year) + "년" + to_string(localTime.tm_mon) + "월"
			+ to_string(localTime.tm_mday) + "일" + to_string(localTime.tm_hour) + "시"
			+ to_string(localTime.tm_min) + "분" + to_string(localTime.tm_sec) + "초" + " / " + message;
		break;
	default:
		return;
	}

	SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
	//MoveScrollbarToEnd(listBox);
	//AdjustListboxHScroll(listBox);
}