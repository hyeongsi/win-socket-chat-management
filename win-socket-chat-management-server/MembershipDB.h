#pragma once
#include <string>

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
public:
	static MembershipDB* GetInstance();
	static void ReleaseInstance();

	int FindIndex(const int kind, const std::string value);
};

