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

class MembershipDB
{
private:
	static MembershipDB* instance;

	MembershipDB();
	vector<string> Split(string input, char delimiter);
	list<string> GetTextArray(string path);
public:
	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	int FindIndex(const int kind, const std::string value);
};

