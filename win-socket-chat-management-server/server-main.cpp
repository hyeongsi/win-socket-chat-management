#include "server-main.h"
#pragma comment(lib,"ws2_32")

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInst;

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
		g_hDlg = hDlg;
		SetWindowPos(hDlg, HWND_TOP, 100, 100, 0, 0, SWP_NOSIZE);

		if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			MessageBox(g_hDlg, "WSAStartup Error", "error", NULL);
			return false;
		}

		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_IP), GetMyIP().c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_PORT), "4567");
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SERVER_SAVE_LOG_BTN:
			for (auto i = 0; i < SendMessage(GetDlgItem(hDlg, IDC_LOG_LIST), LB_GETCOUNT, 0, 0); i++)
			{
				char str[PACKET_SIZE];
				SendMessage(GetDlgItem(hDlg, IDC_LOG_LIST), LB_GETTEXT, i, (LPARAM)str);
				vector<string>writeData;
				writeData.emplace_back(str);
				MembershipDB::GetInstance()->WriteDataToCsv(SAVE_LOG_PATH, writeData);
			}
			
			MessageBox(hDlg, "모든 로그를 저장했습니다.", "로그저장", 0);
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
	switch (MembershipDB::GetInstance()->ExistValue(ID, id))
	{
	case ID:
		return ExsistsSameId;
	case -2:
		return -2;
	}

	switch (MembershipDB::GetInstance()->ExistValue(NAME, name))
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