#include "main.hpp"
#include <luabind/object.hpp>
#include "hook_mgr.hpp"

hook_mgr* g_hook = new hook_mgr;

void hook_mgr::add(string hookName, string uniqueName, const luabind::object &fn) {
	hook_data data;
	data.hook = hookName;
	data.unique = uniqueName;
	data.obj = fn;
	meta[hookName].push_back(data);
}

void hook_mgr::remove(string hookName, string uniqueName) {
	for(size_t i = 0; i < meta[hookName].size(); i++) {
		if(meta[hookName][i].unique == uniqueName) {
			meta[hookName].erase(meta[hookName].begin() + i);
		}
	}
}

void hook_mgr::reload() {
	meta.clear();
}

vector<hook_data> hook_mgr::get_data(string hookName) {
	return meta[hookName];
}

hook_mgr* hook_mgr::singleton() {
	return g_hook;
}
