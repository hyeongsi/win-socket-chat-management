#pragma once
#include <list>
#include <vector>
#include <string>

#include "jsoncppkor\include\json\json.h"
#pragma comment(lib,"jsoncppkor\\json_vc71_libmtd.lib")

using namespace std;

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
	vector<string> Split(string input, char delimiter);

	list<string> GetColumn(std::string str);
public:
	const char* MEMBERSHIIP_DB_PATH = "userData\\membershipData.csv";
	const char* BAN_USER_PATH = "userData\\banUserData.csv";

	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	int ExistValue(const int kind, const std::string value);

	std::string FindName(std::string id);
	int LoginCheck(const string id, const string pw, Json::Value* value);
	bool WriteDataToCsv(const string path, std::vector<std::string> data);
};

