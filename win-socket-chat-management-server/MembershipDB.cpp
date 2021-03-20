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
    // ��ã���� -1 ����, error -2 ����
    list<string> textArray = GetTextArray(membershipDBPath);
    if (textArray.size() <= 1)  // id, pw, name �Ǻ� ���ڿ� ������ ù���� ����
        return -2;  

    int rowNumber;
    rowNumber = kind;

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
            continue;   // ���� ���� ����

        vector<string> row = Split(*loadTextIterator, ',');
        for (int i = 0; i < (int)row.size(); i++)    // id, pw, name
        {
            try
            {
                if (row[rowNumber] == value)
                    return kind;    // ã���� �ش� �ε��� ����
            }
            catch (const std::exception&)   // rowNumber OutOfIndex �߻� �� ó��
            {
                return -2;
            }
        }
    }
    
    return -1;
}
