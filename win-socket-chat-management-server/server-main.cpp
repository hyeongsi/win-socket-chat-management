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

	PostQuitMessage(0);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_INITDIALOG:
		InitDialogMethod(hDlg);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_CONNECT_USER_CHECK_BTN:	// 접속사 수 확인
			CheckConnectUserBtnMethod();
			break;
		case ID_USER_CHECK_BTN:			// 모든 사용자 확인
			CheckUserIdListBtnMethod();
			break;
		case ID_BAN_BUTTON:
			BanBtnMethod();
			break;
		case ID_UNBAN_BUTTON:
			UnBanBtnMethod();
			break;
		case ID_SERVER_SAVE_LOG_BTN:
			SaveServerLogBtnMethod();
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

		switch (recvValue["kind"].asInt())
		{
		case SignUp:
			SignUpMessageMethod(recvValue);
			break;
		case Login:
			LoginMessageMethod(recvValue, &userId, &userName);
			break;
		case ChattingRoomInit:
			ChattingRoomInitMethod(sendValue, &userId, &userName);
			break;
		case GetChattringRoomName:
			GetChattingRoomNameMethod(recvValue, sendValue);
			break;
		case Message:
			JsonMessageMethod(recvValue, &userId, &userName);
			break;
		case SetFileRequest:
			SetFileRequestMessageMethod(recvValue, &userName);
			break;
		case AddFriend:
			AddFriendMessageMethod(recvValue, &userId);
			break;
		case GetFileRequest:
			GetFileRequestMessageMethod(recvValue, &userName);
			break;
		case Emoticon:
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

	clientSocketListMutex.lock();
	clientSocketList.clear();
	clientSocketListMutex.unlock();

	isOpenServer = false;
	DebugLogUpdate(logBox, "서버가 종료되었습니다.");
}

void InitDialogMethod(HWND hDlg)
{
	g_hDlg = hDlg;
	SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		MessageBox(g_hDlg, "WSAStartup Error", "error", NULL);
		return;
	}

	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_IP), GetMyIP().c_str());
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_PORT), "4567");
}

void CheckConnectUserBtnMethod()
{
	SendMessage(GetDlgItem(g_hDlg, ID_CONNECT_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("v"));	// 텍스트 수정
	SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("모든 사용자"));// 텍스트 수정
	SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_RESETCONTENT, 0, 0);	// 기존 데이터 삭제

	clientSocketListMutex.lock();
	for (auto& connectUser : clientSocketList)
		DebugLogUpdate(userBox, "id : " + connectUser.id + " name : " + connectUser.name);
	clientSocketListMutex.unlock();
}

void CheckUserIdListBtnMethod()
{
	SendMessage(GetDlgItem(g_hDlg, ID_CONNECT_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("접속자"));	// 텍스트 수정
	SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("v"));// 텍스트 수정
	SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_RESETCONTENT, 0, 0);

	for (auto& userInfo : MembershipDB::GetInstance()->GetUserInfoList())
		DebugLogUpdate(userBox, "id : " + userInfo.id + " name : " + userInfo.name);
}

void BanBtnMethod()
{
	userIdDataVectorMutex.lock();
	getUserIdData.emplace_back(MembershipDB::GetInstance()->Split(GetUserIdInUserList(), ' ')[2]);	// id 저장
	if (MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->BAN_USER_PATH,
		ID, getUserIdData[0]) >= 0)
	{
		MessageBox(g_hDlg, (getUserIdData[0] + " 밴 실패 동일한 id가 이미 밴 상태입니다.").c_str(), 0, 0);
		getUserIdData.clear();
		userIdDataVectorMutex.unlock();
		return;
	}

	if (MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->BAN_USER_PATH, getUserIdData))
		MessageBox(g_hDlg, (getUserIdData[0] + " 밴 성공").c_str(), 0, 0);
	else
		MessageBox(g_hDlg, (getUserIdData[0] + " 밴 실패").c_str(), 0, 0);

	getUserIdData.clear();
	userIdDataVectorMutex.unlock();
}

void UnBanBtnMethod()
{
	int findIdIndex = 0;
	int count = 0;

	userIdDataVectorMutex.lock();
	getUserIdData.emplace_back(MembershipDB::GetInstance()->Split(GetUserIdInUserList(), ' ')[2]);	// id 저장
	findIdIndex = MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->BAN_USER_PATH,
		ID, getUserIdData[0], true);

	getUserIdData.clear();
	userIdDataVectorMutex.unlock();

	if (findIdIndex >= 0)
	{
		list<string> banUserData = MembershipDB::GetInstance()->GetColumn(
			MembershipDB::GetInstance()->BAN_USER_PATH);

		FILE* fp = fopen(MembershipDB::GetInstance()->BAN_USER_PATH, "w");
		fprintf(fp, "id\n");
		fclose(fp);

		for (auto iterator : banUserData)
		{
			count++;

			if (count == 1)
				continue;
			if (count - 1 == findIdIndex)
				continue;

			userIdDataVectorMutex.lock();
			getUserIdData.clear();
			getUserIdData.emplace_back(iterator);
			MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->BAN_USER_PATH, getUserIdData);
			userIdDataVectorMutex.unlock();
		}

		MessageBox(g_hDlg, "밴 취소 성공", 0, 0);
	}
	else
	{
		MessageBox(g_hDlg, "밴 취소 실패", 0, 0);
	}

	userIdDataVectorMutex.lock();
	getUserIdData.clear();
	userIdDataVectorMutex.unlock();
}

void SaveServerLogBtnMethod()
{
	for (auto i = 0; i < SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST), LB_GETCOUNT, 0, 0); i++)
	{
		char str[PACKET_SIZE];
		SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST), LB_GETTEXT, i, (LPARAM)str);
		vector<string>writeData;
		writeData.emplace_back(str);
		MembershipDB::GetInstance()->WriteDataToCsv(SAVE_LOG_PATH, writeData);
	}

	MessageBox(g_hDlg, "모든 로그를 저장했습니다.", "로그저장", 0);
}

void SignUpMessageMethod(Json::Value recvValue)
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

	if (!SendJsonData(sendValue, clientSocket))
		return;
}

void LoginMessageMethod(Json::Value recvValue , string* userId, string* userName)
{
	Json::Value sendValue;

	sendValue["result"] = MembershipDB::GetInstance()->
		LoginCheck(recvValue["id"].asString(),
			recvValue["pw"].asString(), &sendValue);

	if (!SendJsonData(sendValue, clientSocket))
		return;

	if (LoginSuccess == sendValue["result"].asInt())
	{
		*userId = recvValue["id"].asString();
		*userName = MembershipDB::GetInstance()->FindName(recvValue["id"].asString());

		clientSocketListMutex.lock();
		clientSocketList.emplace_back(UserData(
			clientSocket,
			*userId,
			*userName));
		clientSocketListMutex.unlock();

		CheckConnectUserBtnMethod();
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
		clientSocketListMutex.lock();
		for (const auto& iterator : clientSocketList)
		{
			SendJsonData(sendValue, iterator.socket);
		}
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

void SetFileRequestMessageMethod(Json::Value recvValue, string* userName)
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
			readByteSize = recv(clientSocket, cBuffer, PACKET_SIZE, 0);
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
		for (const auto& iterator : clientSocketList)
		{
			SendJsonData(sendValue, iterator.socket);
		}
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

void AddFriendMessageMethod(Json::Value recvValue, string* userId)
{
	Json::Value sendValue;
	int findReturnValue;

	sendValue["kind"] = AddFriend;
	if (recvValue["friendId"].asString() == *userId)
	{
		sendValue["result"] = false;
		sendValue["message"] = "자기 자신은 친구로 등록할 수 없습니다.";
		SendJsonData(sendValue, clientSocket);
		return;
	}

	findReturnValue = MembershipDB::GetInstance()->ExistValue(
		MembershipDB::GetInstance()->MEMBERSHIIP_DB_PATH, ID, recvValue["friendId"].asString());

	if (findReturnValue < 0)
	{
		sendValue["result"] = false;
		sendValue["message"] = "해당 id는 없는 id 입니다.";
		SendJsonData(sendValue, clientSocket);
		return;
	}

	sendValue["result"] = true;
	SendJsonData(sendValue, clientSocket);
}

void GetFileRequestMessageMethod(Json::Value recvValue, string* userName)
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

		SendJsonData(sendValue, clientSocket);
		SendFileDataToServer(fp, fileSize, clientSocket);
		fclose(fp);
		DebugLogUpdate(logBox, *userName + recvValue["fileName"].asString() + " 파일전송성공");
	}
	else
		DebugLogUpdate(logBox, *userName + recvValue["fileName"].asString() + " 파일전송실패");
}

void ChattingRoomInitMethod(Json::Value sendValue, string* userId, string* userName)
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
		sendValue["roomNumberStr"] = "";
		SendJsonData(sendValue, clientSocket);
		return;
	}
		
	// 채팅방 연결 처리
	for (auto& iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		for (auto i = 1; i != copyRoomNumber.size(); i++)
		{
			if (iterator->GetChattingRoomNumber() == stoi(copyRoomNumber[i]))
			{
				iterator->ConnectChattingRoom(clientSocket);
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
	sendValue["roomNumberStr"] = chattingRoomNumberStr;
	SendJsonData(sendValue, clientSocket);
	return;
}

void GetChattingRoomNameMethod(Json::Value recvValue, Json::Value sendValue)
{
	for (auto iterator : ChattingRoomManager::GetInstance()->GetChattingRoomList())
	{
		if (iterator->GetChattingRoomNumber() == stoi(recvValue["roomNumber"].asString()))
		{
			sendValue["roomName"] = iterator->GetChattingRoomName();
			SendJsonData(sendValue, clientSocket);
			return;
		}
	}
}

void ExitClient(SOCKET* clientSocket)
{
	int count = 0;

	clientSocketListMutex.lock();
	for (auto iterator = clientSocketList.begin(); iterator != clientSocketList.end();)
	{
		if ((*iterator).socket == *clientSocket)
		{
			DebugLogUpdate(logBox, (*iterator).id + "유저 로그아웃");
			clientSocketList.erase(iterator);
			SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_DELETESTRING, count, 0);
			break;
		}
		else
		{
			count++;
			iterator++;
		}
	}
	clientSocketListMutex.unlock();

	closesocket(*clientSocket);
}

string GetMyIP()
{
	char* ip = nullptr;
	char name[255];
	PHOSTENT host;

	if (gethostname(name, sizeof(name)) == 0)
	{
		if ((host = gethostbyname(name)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr*)*host->h_addr_list);
		}
	}

	if (nullptr == ip)
		return "";

	return string(ip);
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

string GetUserIdInUserList()
{
	char tempChatMessage[PACKET_SIZE];
	SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETTEXT,
		SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCURSEL, 0, 0),
		(LPARAM)tempChatMessage);
	strcat(tempChatMessage, "\0");
	//선택중인 인덱스 문자 가져옴

	return tempChatMessage;
}

void MoveScrollbarToEnd(HWND hwnd)
{
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void AdjustListboxHScroll(HWND hwnd)
{
	int nTextLen = 0, nWidth = 0;
	int nCount = 0, idx = 0;
	HDC hDc = NULL;
	HFONT hFont = NULL;
	SIZE sz = { 0 };
	char pszText[MAX_PATH] = { 0, };

	nCount = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
	hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	hDc = GetDC(hwnd);
	SelectObject(hDc, (HGDIOBJ)hFont);

	for (idx = 0; idx < nCount; idx++)
	{
		nTextLen = SendMessage(hwnd, LB_GETTEXTLEN, idx, 0);
		memset(pszText, 0, MAX_PATH);
		SendMessage(hwnd, LB_GETTEXT, idx, (LPARAM)pszText);
		GetTextExtentPoint32A(hDc, pszText, nTextLen, &sz);
		nWidth = max(sz.cx, nWidth);
	}

	ReleaseDC(hwnd, hDc);
	SendMessage(hwnd, LB_SETHORIZONTALEXTENT, nWidth + 20, 0);
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
	MoveScrollbarToEnd(listBox);
	AdjustListboxHScroll(listBox);
}