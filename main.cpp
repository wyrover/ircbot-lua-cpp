#include "main.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <lua5.2/lua.hpp>
#include <mysql++/mysql++.h>
#include <luabind/lua_include.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/iterator_policy.hpp>

#include "irc.hpp"
#include "lua_mgr.hpp"
#include "lua_irc.hpp"

char g_cwd[1024] = {0};

bool makepid() {
	pid_t pid = getpid();
	if(getcwd(g_cwd, sizeof(g_cwd)) != NULL) {
		char pidf[1024] = {0};
		strcpy(pidf, g_cwd);
		strcat(pidf, "/irc.pid");
		FILE* fp = fopen(pidf, "w");
		if(fp) {
			fprintf(fp, "%i", pid);
			fclose(fp);
			return true;
		}
	}
	return false;
}

int main(int argc, char *argv[]) {
	printf("Super kawaii bot starto~\n");
	
	if(makepid() == false) {
		printf("WARNING: Failed to create pid file.\n");
	}
	
	lua_irc::singleton()->set_base_path(g_cwd);
	
	if(lua_mgr::singleton()->init() == false) {
		printf("Could not initialize lua!\n");
		return 0;
	}
	
	// Note: When the connection closes, this function will finish execution
	lua_irc::singleton()->start();
	
	printf("Bye~\n");
	
	return 0;
}
