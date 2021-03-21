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

int MembershipDB::FindIndex(const int kind, const std::string value)
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

    if (textArray.size() <= 1)  // id, pw, name �Ǻ� ���ڿ� ������ ù���� ����
        return -2;  

    int rowNumber;
    rowNumber = kind;

    for (auto loadTextIterator = textArray.begin(); loadTextIterator != textArray.end(); )
    {
        if (loadTextIterator == textArray.begin())
        {
            loadTextIterator++;
            continue;   // ���� ���� ����
        }

        vector<string> row = Split(*loadTextIterator, ',');
        if (row[rowNumber] == value)
            return kind;    // ã���� �ش� �ε��� ����

        row.clear();
        loadTextIterator++;
    }
    
    return -1;
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
