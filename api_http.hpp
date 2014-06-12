#pragma once

class api_http {
public:
	luabind::object query(string url, luabind::object options);
	luabind::object last_error();
	
	static api_http* singleton();

private:
};
