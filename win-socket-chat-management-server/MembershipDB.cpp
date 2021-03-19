#include "MembershipDB.h"

MembershipDB* MembershipDB::instance = nullptr;
MembershipDB::MembershipDB() { }

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
    switch (kind)
    {
    case ID:
        break;
    case PW:
        break;
    case NAME:
        break;
    }

    // 찾으면 해당 인덱스 리턴

    // 못찾으면 -1 리턴
    return -1;
}
