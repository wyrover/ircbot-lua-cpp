#include "main.hpp"

#include <lua5.2/lua.hpp>
#include <luabind/lua_include.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/iterator_policy.hpp>
#include <mysql++/mysql++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "hook_mgr.hpp"
#include "irc.hpp"
#include "lua_irc.hpp"
#include "api_mysql.hpp"
#include "api_http.hpp"

#include "lua_mgr.hpp"

lua_mgr* g_lua = new lua_mgr;

bool lua_mgr::init() {
	l_state = luaL_newstate();

	if(l_state == NULL) return false;

	luaL_openlibs(l_state);
	luabind::open(l_state);

	luabind::module(l_state) [
		luabind::class_<irc_message>("irc_message")
			.def_readonly("preop", &irc_message::preop)
			.def_readonly("hostmask", &irc_message::hostmask)
			.def_readonly("opcode", &irc_message::opcode)
			.def_readonly("channel", &irc_message::channel)
			.def_readonly("args", &irc_message::args)
			.def_readonly("nick", &irc_message::nick)
			.def_readonly("user", &irc_message::user)
			.def_readonly("host", &irc_message::host)
			.def_readonly("raw", &irc_message::raw),
			
		luabind::class_<hook_mgr>("hook_mgr")
			.def("add", &hook_mgr::add)
			.def("remove", &hook_mgr::remove),
	
		// IRC Class
		luabind::class_<lua_irc>("lua_irc")
			// irc functions
			.def("config", &lua_irc::get_config)
			.def("send", &lua_irc::push)
			.def("nick", &lua_irc::set_nick)
			.def("identify", &lua_irc::identify)
			.def("join", &lua_irc::join_channel)
			.def("privmsg", &lua_irc::privmsg)
			.def("notice", &lua_irc::notice)
			.def("version", &lua_irc::version)
			.def("ghost", &lua_irc::ghost)
			.def("quit", &lua_irc::quit)
			.def("bold", &lua_irc::bold)
			.def("color", &lua_irc::color)
			.def("underline", &lua_irc::underline)
			.def("script_name", &lua_irc::script_name)
			.def("script_path", &lua_irc::script_path)
			.def("script_num", &lua_irc::script_num),
					
		luabind::class_<api_mysql>("api_mysql")
			.def("connect", &api_mysql::connect)
			.def("connected", &api_mysql::connected)
			.def("disconnect", &api_mysql::disconnect)
			.def("count_rows", &api_mysql::count_rows)
			.def("query", &api_mysql::query)
			.def("array_query", &api_mysql::array_query)
			.def("affected_query", &api_mysql::affected_query)
			.def("client_version", &api_mysql::client_version)
			.def("protocol_version", &api_mysql::protocol_version),
			
		luabind::class_<api_http>("api_http")
			.def("query", &api_http::query)
	];
	
	// Expose API interface
	luabind::globals(l_state)["hook"] = hook_mgr::singleton();
	luabind::globals(l_state)["irc"] = lua_irc::singleton();
	luabind::globals(l_state)["mysql"] = api_mysql::singleton();
	luabind::globals(l_state)["http"] = api_http::singleton();
	
	return true;
}

bool lua_mgr::load_file(string file) {
	bool r = false;
	try {
		r = (luaL_dofile(l_state, file.c_str()) == 0);
	} catch(luabind::error &e) {
		printf("Lua error [%s]: %s\n", file.c_str(), e.what());
		printf("Specific: %s\n", lua_tostring(l_state, -1));
	}
	return r;
}

bool lua_mgr::reload() {
	this->close();
	l_state = NULL;
	return this->init();
}

void lua_mgr::close() {
	lua_close(l_state);
}

string lua_mgr::error() {
	return lua_tostring(l_state, -1);
}

lua_mgr* lua_mgr::singleton() {
	return g_lua;
}
