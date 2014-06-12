#pragma once

// It will be fairly trivial to make this windows compatible.
// but for now, I am not going to do it, lol

struct thread_info_t {
	lua_State* L;
	luabind::object func;
	luabind::object params;
	pthread_t handle;
	size_t index;
};

class api_thread {
public:
	api_thread() : terminate_switch(false) {}

	bool start(luabind::object const& func, luabind::object const &params);
	int sleep(unsigned int s);
	
	// Internal functions used for the lua stuff
	size_t count();
	thread_info_t* info(size_t idx);
	bool active(size_t thread_idx);
	void terminate(bool state);
	bool should_terminate();
	void clear();
	
	static api_thread* singleton();
	
private:
	bool terminate_switch;
	vector<thread_info_t> threads;
};
