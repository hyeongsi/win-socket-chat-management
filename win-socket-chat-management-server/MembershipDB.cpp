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

int MembershipDB::ExistValue(const int kind, const std::string value)
{
    list<string> textArray = GetColumn(MEMBERSHIIP_DB_PATH);
    
    if (textArray.size() <= 1)  // id, pw, name 판별 문자열 때문에 첫줄은 제외
        return -2;  

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // 위와 같은 문제
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[kind] == value)
            return kind;    // 찾으면 해당 인덱스 리턴

        row.clear();
        loadTextIterator++;
    }
    
    return -1;
}

std::string MembershipDB::FindName(std::string id)
{
    list<string> textArray = GetColumn(MEMBERSHIIP_DB_PATH);

    if (textArray.size() <= 1)  // id, pw, name 판별 문자열 때문에 첫줄은 제외
        return "";

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // 위와 같은 문제
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
            continue;   // 첫줄 제외
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[0] == id)
        {
            (*value)["message"] = row[1] + "사유로 밴이 당했습니다.";
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
            continue;   // 첫줄 제외
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[0] == id && row[1] == pw)
        {
            (*value)["message"] = "id 혹은 pw가 잘못되었습니다.";
             return LoginSuccess;
        }  

        row.clear();
        loadTextIterator++;
    }

    (*value)["message"] = "id 혹은 pw가 잘못되었습니다.";
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
            if ((iterator + 1) == data.end())   // 끝이라면
            {
                foutput<< (*iterator) << endl;
            }
            else
            {
                foutput << (*iterator) << ",";
            }
        }
    }

    foutput.close();

    return false;
}


