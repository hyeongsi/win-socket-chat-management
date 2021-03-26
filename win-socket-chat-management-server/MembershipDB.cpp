#include "MembershipDB.h"
#include <vector>
#include <sstream>
#include <fstream>

MembershipDB* MembershipDB::instance = nullptr;
MembershipDB::MembershipDB() { }

vector<string> MembershipDB::Split(string input, char delimiter)
{
    vector<string> row;
    stringstream ss(input);
    string buffer;

    while (getline(ss, buffer, delimiter))
    {
        row.emplace_back(buffer);
    }

    return row;
}

MembershipDB* MembershipDB::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new MembershipDB();
    }

    return instance;
}

void MembershipDB::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

list<string> MembershipDB::GetColumn(string str)
{
    list<string> textArray;
    ifstream finput;
    finput.open(str);
    
    if (finput.is_open())
    {
        string str;
        while (true)
        {
            getline(finput, str);
            if (finput.eof())
                break;
            textArray.emplace_back(str);
        }
    }
    finput.close();

    return textArray;
}

int MembershipDB::ExistValue(string str, const int kind, const string value)
{
    list<string> textArray = GetColumn(str);
    
    if (textArray.size() <= 1)  // id, pw, name �Ǻ� ���ڿ� ������ ù���� ����
        return -2;  

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ���� ���� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[kind] == value)
            return kind;    // ã���� �ش� �ε��� ����

        row.clear();
        loadTextIterator++;
    }
    
    return -1;
}

std::list<UserInfo> MembershipDB::GetUserInfoList()
{
    list<UserInfo> userInfoList;
    list<string> textArray = GetColumn(MEMBERSHIIP_DB_PATH);

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ù�� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        userInfoList.emplace_back(row[0], row[2]);

        row.clear();
        loadTextIterator++;
    }

    return userInfoList;
}

std::string MembershipDB::FindName(string id)
{
    list<string> textArray = GetColumn(MEMBERSHIIP_DB_PATH);

    if (textArray.size() <= 1)  // id, pw, name �Ǻ� ���ڿ� ������ ù���� ����
        return "";

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ���� ���� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[0] == id)
        {
            return row[2];
        } 

        row.clear();
        loadTextIterator++;
    }

    return "";
}

int MembershipDB::LoginCheck(const string id, const string pw, Json::Value* value)
{
    list<string> textArray = GetColumn(BAN_USER_PATH);
    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ù�� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[0] == id)
        {
            return Ban;
        }

        row.clear();
        loadTextIterator++;
    }

    textArray = GetColumn(MEMBERSHIIP_DB_PATH);

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ù�� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[0] == id && row[1] == pw)
        {
            (*value)["message"] = "id Ȥ�� pw�� �߸��Ǿ����ϴ�.";
             return LoginSuccess;
        }  

        row.clear();
        loadTextIterator++;
    }

    (*value)["message"] = "id Ȥ�� pw�� �߸��Ǿ����ϴ�.";
    return WringIdOrPassword;
}

bool MembershipDB::WriteDataToCsv(const string path,  vector<string> data)
{
    ofstream foutput;
    foutput.open(path, ios::app);

    if (foutput.is_open())
    {
        for (auto iterator = data.begin(); iterator != data.end(); iterator++)
        {
            if ((iterator + 1) == data.end())   // ���̶��
            {
                foutput<< (*iterator) << endl;
            }
            else
            {
                foutput << (*iterator) << ",";
            }
        }
        foutput.close();
        return true;
    }

    foutput.close();

    return false;
}


