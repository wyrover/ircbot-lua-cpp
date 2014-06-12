#include "main.hpp"
#include <luabind/object.hpp>
#include <luabind/function.hpp>
#include "lua_mgr.hpp"
#include "api_thread.hpp"
#include <signal.h>

api_thread* g_thread = new api_thread;

/* WARNING: THIS WHOLE THING IS BROKEN, BECAUSE S0BEIT DIDNT KNOW YOU CANT DO MULTITHREADING WITH LUA */
/* LOL EVERYONE LAUGH AT HIS MISFORTUNE */
/* Seriously though, this will be replaced by executing queued functions in the main thread loop, it just isn't done yet */

void* lua_thread_func(void* p) {
	printf("Lua Thread!\n");
	thread_info_t* ti = (thread_info_t*) api_thread::singleton()->info(((size_t*) p)[0]);
	if(ti == NULL){ printf("NULL param, how does this happen?\n"); return NULL; } //Well, shit
	printf("Setting Luabind state thing\n");
	luabind::open(ti->L); // I don't know if this is a terrible idea
	printf("Calling lua function!\n");
	luabind::call_function<void>(ti->func, ti->params);
}

bool api_thread::start(luabind::object const& func, luabind::object const &params) {
	if(terminate_switch) return false;
	
	thread_info_t p;
	p.func = func;
	p.params = params;
	p.index = threads.size();
	p.L = lua_newthread(lua_mgr::singleton()->state());
	
	printf("Thread (%i) -> Create\n", p.index);
	
	if(pthread_create(&p.handle, NULL, lua_thread_func, &p.index)) {
		return false;
	}
	
	printf("Adding thread to list!\n");
	
	threads.push_back(p);
	
	printf("Removing initially allocated thread information...\n");
	
	return true;
}

// We won't need usleep for win32, and we don't realistically need it at all for lua
// So there is no exported usleep function.
int api_thread::sleep(unsigned int s) {
	return ::usleep(s * 1000);
}

size_t api_thread::count() {
	return threads.size();
}

thread_info_t* api_thread::info(size_t idx) {
	return &threads[idx];
}

bool api_thread::active(size_t idx) {
	if(idx > count() || idx < 0) {
		return false;
	}
	
	return (pthread_kill(threads[idx].handle, 0) == 0);
}

void api_thread::terminate(bool state) {
	terminate_switch = state;
}

bool api_thread::should_terminate() {
	return terminate_switch;
}

void api_thread::clear() {
	printf("Clearing threads...\n");
	threads.clear();
}

api_thread* api_thread::singleton() {
	return g_thread;
}
