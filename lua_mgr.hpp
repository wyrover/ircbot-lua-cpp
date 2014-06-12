#pragma once

class lua_mgr {
public:
	bool init();
	bool load_file(string file);
	lua_State* state(){ return l_state; }
	bool reload();
	void close();
	string error();

	static lua_mgr* singleton();

private:
	lua_State* l_state;
};
