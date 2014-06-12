#pragma once

struct hook_data {
	string 				hook;
	string 				unique;
	luabind::object 	obj;
};

class hook_mgr {
public:
	void add(string hookName, string uniqueName, const luabind::object &fn);
	void remove(string hookName, string uniqueName);
	void reload();
	vector<hook_data> get_data(string hookName);

	static hook_mgr* singleton();

private:
	hash_map<string, vector<hook_data>> meta;
};
