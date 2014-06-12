#include "main.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <luabind/lua_include.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>
#include <luabind/iterator_policy.hpp>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "hook_mgr.hpp"
#include "irc.hpp"
#include "lua_irc.hpp"
#include "lua_mgr.hpp"

#define LUAFUNC_ASSERT(x) \
	try{ x; } catch(luabind::error &e) { \
		printf("Lua error [%s][%s]: %s\n", \
			hooks[i].unique.c_str(), \
			hooks[i].hook.c_str(), \
			e.what()); \
			printf("Specific: %s\n", lua_tostring(lua_mgr::singleton()->state(), -1)); \
		}

lua_irc* g_irc = new lua_irc;

lua_irc* lua_irc::singleton() {
	return g_irc;
}

void lua_irc::on_start() {
	for(size_t i = 0; i < scripts.all.size(); i++) {
		delete scripts.all[i];
	}
	
	scripts.all.clear();
	
	// We're going to load config here
	lua_mgr::singleton()->load_file(base_path + "/irc.lua");
	
	// Now the modules
	string mod_dir = this->base_path + "/lua/";

	DIR* d = opendir(mod_dir.c_str());

	if(d) {
		dirent* dir = NULL;

		while((dir = readdir(d)) != NULL) {
			if(dir->d_name[0] == '.' || dir->d_name[strlen(dir->d_name)-1] == '~') { continue; }

			struct stat fi;
			
			lstat((mod_dir + dir->d_name).c_str(), &fi);

			if(!(fi.st_mode & S_IFREG)) { continue; }

			if(lua_mgr::singleton()->load_file(mod_dir + dir->d_name) == false) {
				printf("Lua LoadFile Error on [%s]: %s\n", dir->d_name, lua_mgr::singleton()->error().c_str());
				exit(0);
			} else {
				lua_irc_script* l = new lua_irc_script;
				l->name = dir->d_name;
				l->path = mod_dir;
				scripts.all.push_back(l);
			}
		}
		closedir(d);
	}
	
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("start");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj));
	}
}

void lua_irc::on_message() {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("RAW_" + message.opcode);
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj));
	}
}

void lua_irc::on_privmsg(string channel, string nick, string user, string text) {
	// Needs additional auth
	if(nick == get_config("irc_root_user")) {
		if(text == "!reload") {
			if(reload()) {
				privmsg(channel, "Reloaded lua!");
			} else {
				privmsg(channel, "Failed to reload lua!");
			}
		}
	}
	
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("privmsg");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, channel, nick, user, text));
	}
}

void lua_irc::on_notice(string from, string to, string text) {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("notice");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, from, to, text));
	}
}

void lua_irc::on_mode(string channel, string target, string modes) {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("mode");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, channel, target, modes));
	}
}

void lua_irc::on_join(string channel, string nick) {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("join");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, channel, nick));
	}
}

void lua_irc::on_part(string channel, string nick) {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("part");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, channel, nick));
	}
}

void lua_irc::on_nick(string from, string to) {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("nick");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj, from, to));
	}
}

void lua_irc::on_ping() {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("ping");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj));
	}
}

void lua_irc::on_update() {
	vector<hook_data> hooks = hook_mgr::singleton()->get_data("update");
	
	for(size_t i = 0; i < hooks.size(); i++) {
		LUAFUNC_ASSERT(luabind::call_function<void>(hooks[i].obj));
	}
}

string lua_irc::get_config(string var) {
	return luabind::object_cast<string>(luabind::globals(lua_mgr::singleton()->state())["config"][var]);
}

// WARNING: If you try to turn this into a function which is in any way invoked by lua,
// it causes some serious issues. I've had stack corruption and stuff, I have no goddamn idea.
// I understand it crashing if I invoke it from lua directly, duh,
// but even when I made a boolean switch and reloaded it after execution it somehow caused issues
// beware.
bool lua_irc::reload() {
	hook_mgr::singleton()->reload();
	if(lua_mgr::singleton()->reload()) {
		this->on_start();
		return true;
	}
	return false;
}
