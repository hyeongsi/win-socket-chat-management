#include "ServerUI.h"
#include "Server.h"

ServerUI* ServerUI::instance = nullptr;
ServerUI::ServerUI() { g_hDlg = NULL; }

ServerUI* ServerUI::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new ServerUI();
    }

    return instance;
}

void ServerUI::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

string ServerUI::GetMyIP()
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

string ServerUI::GetUserIdInUserList()
{
    char tempChatMessage[PACKET_SIZE];
    SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETTEXT,
        SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCURSEL, 0, 0),
        (LPARAM)tempChatMessage);
    strcat(tempChatMessage, "\0");
    //�������� �ε��� ���� ������

    return tempChatMessage;
}

void ServerUI::MoveScrollbarToEnd(HWND hwnd)
{
    SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void ServerUI::AdjustListboxHScroll(HWND hwnd)
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

void ServerUI::InitDialogMethod(HWND hDlg)
{
    g_hDlg = hDlg;

    SetWindowPos(hDlg, HWND_TOP, 200, 100, 0, 0, SWP_NOSIZE);
    // ȭ�� ���� ��ġ ����

    if (0 != WSAStartup(MAKEWORD(2, 2), &Server::GetInstance()->wsaData))
    {
        MessageBox(g_hDlg, "WSAStartup Error", "error", NULL);
        return;
    }

    // ip, port UI ���
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_IP), GetMyIP().c_str());
    SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MY_PORT), to_string(PORT).c_str());
}

void ServerUI::CheckConnectUserBtnMethod()
{
    SendMessage(GetDlgItem(g_hDlg, ID_CONNECT_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("v"));	// �ؽ�Ʈ ����
    SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("��� �����"));// �ؽ�Ʈ ����
    SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_RESETCONTENT, 0, 0);	// ���� ������ ����

    Server::GetInstance()->clientSocketListMutex.lock();
    for (auto& connectUser : Server::GetInstance()->clientSocketList)
        DebugLogUpdate(userBox, "id : " + connectUser.id + " name : " + connectUser.name);
    Server::GetInstance()->clientSocketListMutex.unlock();
}

void ServerUI::CheckUserIdListBtnMethod()
{
    SendMessage(GetDlgItem(g_hDlg, ID_CONNECT_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("������"));	// �ؽ�Ʈ ����
    SendMessage(GetDlgItem(g_hDlg, ID_USER_CHECK_BTN), WM_SETTEXT, 0, (LPARAM)("v"));// �ؽ�Ʈ ����
    SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_RESETCONTENT, 0, 0);

    for (auto& userInfo : MembershipDB::GetInstance()->GetUserInfoList())
        DebugLogUpdate(userBox, "id : " + userInfo.id + " name : " + userInfo.name);
}


void ServerUI::DebugLogUpdate(int kind, string message)
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

        logMessage = to_string(1900 + localTime.tm_year) + "��" + to_string(localTime.tm_mon) + "��"
            + to_string(localTime.tm_mday) + "��" + to_string(localTime.tm_hour) + "��"
            + to_string(localTime.tm_min) + "��" + to_string(localTime.tm_sec) + "��" + " / " + message;
        break;
    default:
        return;
    }

    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)logMessage.c_str());
    MoveScrollbarToEnd(listBox);
    AdjustListboxHScroll(listBox);
}

void ServerUI::BanBtnMethod()
{
    if (-1 == SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCURSEL, 0, 0))
        return;

    vector<string> getUserIdData;

    getUserIdData.emplace_back(MembershipDB::GetInstance()->Split(GetUserIdInUserList(), ' ')[2]);	// id ����
    if (MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->BAN_USER_PATH,
        ID, getUserIdData[0]) >= 0)
    {
        MessageBox(g_hDlg, (getUserIdData[0] + " �� ���� ������ id�� �̹� �� �����Դϴ�.").c_str(), 0, 0);
        return;
    }

    if (MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->BAN_USER_PATH, getUserIdData))
        MessageBox(g_hDlg, (getUserIdData[0] + " �� ����").c_str(), 0, 0);
    else
        MessageBox(g_hDlg, (getUserIdData[0] + " �� ����").c_str(), 0, 0);
}

void ServerUI::UnBanBtnMethod()
{
    if (-1 == SendMessage(GetDlgItem(g_hDlg, IDC_USERS_LIST), LB_GETCURSEL, 0, 0))
        return;

    int findIdIndex = 0;
    int count = 0;

    vector<string> getUserIdData;

    getUserIdData.emplace_back(MembershipDB::GetInstance()->Split(GetUserIdInUserList(), ' ')[2]);	// id ����
    findIdIndex = MembershipDB::GetInstance()->ExistValue(MembershipDB::GetInstance()->BAN_USER_PATH,
        ID, getUserIdData[0], true);

    getUserIdData.clear();

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

            getUserIdData.emplace_back(iterator);
            MembershipDB::GetInstance()->WriteDataToCsv(MembershipDB::GetInstance()->BAN_USER_PATH, getUserIdData);
        }

        MessageBox(g_hDlg, "�� ��� ����", 0, 0);
    }
    else
    {
        MessageBox(g_hDlg, "�� ��� ����", 0, 0);
    }
}

void ServerUI::SaveServerLogBtnMethod()
{
    for (auto i = 0; i < SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST), LB_GETCOUNT, 0, 0); i++)
    {
        char str[PACKET_SIZE];
        SendMessage(GetDlgItem(g_hDlg, IDC_LOG_LIST), LB_GETTEXT, i, (LPARAM)str);
        vector<string>writeData;
        writeData.emplace_back(str);
        MembershipDB::GetInstance()->WriteDataToCsv(SAVE_LOG_PATH, writeData);
    }

    MessageBox(g_hDlg, "��� �α׸� �����߽��ϴ�.", "�α�����", 0);
}
