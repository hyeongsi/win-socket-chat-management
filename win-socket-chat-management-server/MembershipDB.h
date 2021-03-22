#pragma once
#include <list>
#include <vector>
#include <string>

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
};

class MembershipDB
{
private:
	static MembershipDB* instance;

	MembershipDB();
	vector<string> Split(string input, char delimiter);

	list<string> GetColumn();
public:
	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	int ExistValue(const int kind, const std::string value);

	std::string FindName(std::string id);
	int LoginCheck(const string id, const string pw);
	bool WriteMembershipData(const string id, const string pw, const string name);
};

