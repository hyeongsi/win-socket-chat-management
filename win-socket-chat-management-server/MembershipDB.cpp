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

list<string> MembershipDB::GetTextArray(string path)
{
    list<string> textArray;
    ifstream fopen;
    fopen.open(path);

    if (fopen.is_open())
    {
        string textStr;
        while (!fopen.eof())
        {
            getline(fopen, textStr);
            textArray.emplace_back(textStr);
        }
    }
    fopen.close();

    return textArray;
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

int MembershipDB::FindIndex(const int kind, const std::string value)
{
    // 못찾으면 -1 리턴, error -2 리턴
    list<string> textArray = GetTextArray(membershipDBPath);
    if (textArray.size() <= 1)  // id, pw, name 판별 문자열 때문에 첫줄은 제외
        return -2;  

    int rowNumber;
    rowNumber = kind;

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
            continue;   // 위와 같은 문제

        vector<string> row = Split(*loadTextIterator, ',');
        for (int i = 0; i < (int)row.size(); i++)    // id, pw, name
        {
            try
            {
                if (row[rowNumber] == value)
                    return kind;    // 찾으면 해당 인덱스 리턴
            }
            catch (const std::exception&)   // rowNumber OutOfIndex 발생 시 처리
            {
                return -2;
            }
        }
    }
    
    return -1;
}
