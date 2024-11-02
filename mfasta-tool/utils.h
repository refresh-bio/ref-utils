#pragma once

#include <string>

using namespace std;

inline string build_new_id(const string& id, const string& prefix)
{
	string s;

	s.push_back(id.front());
	s.insert(s.end(), prefix.begin(), prefix.end());
	s.insert(s.end(), id.begin() + 1, id.end());

	return s;
}