#include "ChattingRoomManager.h"

ChattingRoomManager* ChattingRoomManager::instance = nullptr;
ChattingRoomManager::ChattingRoomManager() 
{
    list<string> chattingroomListInfo = 
        MembershipDB::GetInstance()->GetColumn(CHATTINGROOM_INFO_PATH);

    for (auto loadTextIterator = chattingroomListInfo.begin(); loadTextIterator != chattingroomListInfo.end(); )
    {
        if (loadTextIterator == chattingroomListInfo.begin())
        {
            loadTextIterator++;
            continue;   // 첫줄 제외
        }

        vector<string> row = MembershipDB::GetInstance()->Split(*loadTextIterator, ',');
        chattingRoomList.emplace_back(new ChattingRoom(stoi(row[0]), row[1]));

        row.clear();
        loadTextIterator++;
    }
}

ChattingRoomManager::~ChattingRoomManager()
{
    for (auto iterator : chattingRoomList)
    {
        delete iterator;
    }

    chattingRoomList.clear();
}

ChattingRoomManager* ChattingRoomManager::GetInstance()
{
    if (nullptr == instance)
    {
        instance = new ChattingRoomManager();
    }

    return instance;
}

void ChattingRoomManager::ReleaseInstance()
{
    delete instance;
    instance = nullptr;
}

list<ChattingRoom*> ChattingRoomManager::GetChattingRoomList()
{
    return chattingRoomList;
}
