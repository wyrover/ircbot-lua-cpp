#include "main.hpp"
#include <luabind/object.hpp>
#include <mysql++/mysql++.h>
#include <lua5.2/lua.hpp>
#include "lua_mgr.hpp"
#include "api_mysql.hpp"

api_mysql* g_mysql = new api_mysql;

bool api_mysql::connect(string host, string username, string password, string db) {
	conn = new mysqlpp::Connection(false); // Memory leak, but testing!
	
	return conn->connect(db.c_str(), host.c_str(), username.c_str(), password.c_str());
}

bool api_mysql::connected() {
	return conn->connected();
}

void api_mysql::disconnect() {
	delete conn;
	conn->disconnect();
}

string api_mysql::error() {
	return conn->error();
}

uint64_t api_mysql::count_rows(string table) {
	return conn->count_rows(table);
}

bool api_mysql::query(string qu) {
	mysqlpp::Query t = conn->query(qu);
	return t.exec();
}

luabind::object api_mysql::array_query(string qu) {
	luabind::object r = luabind::newtable(lua_mgr::singleton()->state());
	mysqlpp::Query t = conn->query(qu);
	if(mysqlpp::StoreQueryResult res = t.store()) {
		for(uint64_t i = 0; i < res.num_rows(); i++) {
			luabind::object res_table = luabind::newtable(lua_mgr::singleton()->state());
			for(uint64_t p = 0; p < res[i].size(); p++) {
				res_table[res.fetch_field(p).name()] = ((string)res[i][p]);
			}
			r[i+1] = res_table;
		}
	}
	return r;
}

uint64_t api_mysql::affected_query(string qu) {
	mysqlpp::Query t = conn->query(qu);
	if(mysqlpp::StoreQueryResult res = t.store()) {
		return t.affected_rows();
	}
	return 0;
}

string api_mysql::client_version() {
	return conn->client_version();
}

int api_mysql::protocol_version() {
	return conn->protocol_version();
}

api_mysql* api_mysql::singleton() {
	return g_mysql;
}
