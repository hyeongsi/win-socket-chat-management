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
	WringIdOrPassword,
	Cancel,
	Ban,
};

class MembershipDB
{
private:
	static MembershipDB* instance;

	MembershipDB();
	
	list<string> GetColumn(std::string str);
public:
	const char* MEMBERSHIIP_DB_PATH = "userData\\membershipData.csv";
	const char* BAN_USER_PATH = "userData\\banUserData.csv";

	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	vector<string> Split(string input, char delimiter);

	int ExistValue(string str, const int kind, const string value);

	std::list<UserInfo> GetUserInfoList();
	std::string FindName(string id);
	int LoginCheck(const string id, const string pw, Json::Value* value);
	bool WriteDataToCsv(const string path, std::vector<string> data);
};

