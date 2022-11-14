#pragma once
#include <list>
#include <vector>
#include <string>

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\lib_json.lib")

using namespace std;

typedef struct UserInfo
{
	string id;
	string name;
	UserInfo(string _id, string _name)
		: id(_id), name(_name) {};

}userInfo;
enum MembershipKey
{
	ID = 0,
	PW = 1,
	NAME = 2,
};

enum LoginFailed
{
	LoginSuccess = 0,
	WrongIdOrPassword = 1,
	Cancel = 2,
	Ban = 3,
};

class MembershipDB
{
private:
	static MembershipDB* instance;

	MembershipDB();
	
public:
	const char* MEMBERSHIIP_DB_PATH = "userData\\membershipData.csv";
	const char* BAN_USER_PATH = "userData\\banUserData.csv";
	const char* FRIEND_LIST_PATH = "userData\\friendListData.csv";

	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	list<string> GetColumn(std::string str);
	vector<string> Split(string input, char delimiter);

	int ExistValue(string str, const int kind, const string value, bool returnIndexNumber = false);

	list<UserInfo> GetUserInfoList();
	string FindName(string id);
	int LoginCheck(const string id, const string pw, Json::Value* value);
	bool WriteDataToCsv(const string path, std::vector<string> data);
};

