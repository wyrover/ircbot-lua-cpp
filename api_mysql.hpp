#pragma once

class api_mysql {
public:
	//
	bool connect(string host, string username, string password, string db);
	bool connected();
	void disconnect();
	string error();
	uint64_t count_rows(string table);
	
	bool query(string qu);
	luabind::object array_query(string qu);
	uint64_t affected_query(string qu);

	//
	string client_version();
	int protocol_version();
	
	//
	static api_mysql* singleton();
	
private:
	mysqlpp::Connection* conn;
};
