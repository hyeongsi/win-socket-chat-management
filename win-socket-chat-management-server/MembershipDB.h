#pragma once
#include <list>
#include <vector>
#include <string>

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\json_vc71_libmtd.lib")

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
	ID,
	PW,
	NAME,
};

enum LoginFailed
{
	LoginSuccess,
	WrongIdOrPassword,
	Cancel,
	Ban,
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

