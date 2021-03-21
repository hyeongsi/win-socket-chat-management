#include "MembershipDB.h"
#include <sstream>
#include <fstream>

const char* membershipDBPath = "membershipData.csv";

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

list<string> MembershipDB::GetColumn()
{
    list<string> textArray;
    ifstream finput;
    finput.open(membershipDBPath);

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
    list<string> textArray = GetColumn();
    
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

int MembershipDB::LoginCheck(const string id, const string pw)
{
    list<string> textArray = GetColumn();

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
             return LoginSuccess;
        }  

        row.clear();
        loadTextIterator++;
    }

    return WringIdOrPassword;
}

bool MembershipDB::WriteMembershipData(const string id, const string pw, const string name)
{
    ofstream foutput;
    foutput.open(membershipDBPath, ios::app);

    if (foutput.is_open())
    {
        foutput << id << "," << pw << "," << name << endl;
    }

    foutput.close();

    return false;
}
