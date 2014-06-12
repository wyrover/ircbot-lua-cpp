#pragma once

// The reason why there's so many wrappers for vectors here is because luabind doesn't accept vectors
// Sorry. LIFE IS FULL OF SURPRISES! ADVENTURE MOTHERFUCKER.
// np: Home Made Kazoku - Home Sweet Home [ http://youtube.com/watch?v=se8V9nQ13HY ]

struct irc_command {
	irc_command(vector<string> input) { 
		this->data = input; 
	}
	
	string get(size_t i) { 
		return this->data[i]; 
	}

	bool isset(size_t i) { 
		if(i >= 0 && i <= size()){ return true; } return false; 
	}

	size_t size() { return this->data.size(); }
	string& operator[](int i) { return this->data[i]; }
	vector<string> data;
};

struct irc_user {
	irc_user() : nick(""), permission('*') {}
	
	irc_user(string name, char perm) : nick(name), permission(perm) {}

	bool valid() {
		return (!nick.empty() && permission != '*');
	}
	
	string nick;
	char permission;
};

struct irc_userlist {
	irc_user operator[](size_t i) { 
		return get(i); 
	}
	
	irc_user get(size_t i){
		if(i < 0 || i > size()){ 
			return irc_user(); 
		}
		
		return this->users[i]; 
	}
	
	irc_user find(string name){ 
		for(size_t i = 0; i < users.size(); i++) {
			if(get(i).nick == name){ 
				return get(i);
			}
		}
		return irc_user();
	}
	
	size_t size(){ 
		return users.size(); 
	}
	
	void add_user(string name, char permission) {
		users.push_back(irc_user(name, permission));
	}
	
	void remove_user(string name) {
		for(size_t i = 0; i < size(); i++) {
			if(get(i).nick == name) {
				users.erase(users.begin() + i);
			}
		}
	}

	vector<irc_user> users;
};

struct irc_channel {
	string 			name;
	irc_userlist 	users;
};

struct irc_message {
	string preop;
	string hostmask;
	string opcode;
	string channel;
	string args;
	string nick;
	string user;
	string host;
	string raw;
	bool error;
	
	void clear() {
		preop.clear();
		hostmask.clear();
		opcode.clear();
		channel.clear();
		args.clear();
		nick.clear();
		user.clear();
		host.clear();
		raw.clear();
		error = false;
	}
};

class irc {
public:
	void start();
	
	void push(string data);
	void set_nick(string nickname);
	void identify();
	void join_channel(string channel, string key);
	void join_channels(vector<string> channels);
	void privmsg(string to, string msg);
	void notice(string to, string msg);
	void version();
	void ghost(string user, string pass);
	void quit(string msg);
	string bold(string text);
	string color(string text, string color);
	string underline(string text);
	
	bool parse_message();
	void message_handler();
	
	// Use this when our nick is taken
	string random_string(int length);

	// Make these!
	// Note: Although I don't include things like hostmask etc in params, since it's accessible by using 'messages' i didn't see the need
	virtual void on_start() = 0;
	virtual void on_message() = 0;
	virtual void on_privmsg(string channel, string nick, string user, string text) = 0;
	virtual void on_notice(string from, string to, string text) = 0;
	virtual void on_mode(string channel, string target, string modes) = 0;
	virtual void on_join(string channel, string nick) = 0;
	virtual void on_part(string channel, string nick) = 0;
	virtual void on_nick(string from, string to) = 0;
	virtual void on_ping() = 0;
	virtual void on_update() = 0;
	
	//
	virtual string get_config(string var) = 0; // This will be filled by the child class

public:
	int 							sock;
	struct sockaddr_in 				sockaddr;
	FILE* 							sock_fp;
	bool	 						thread_active;
	string 							buffer;
	irc_message 					message;

protected:
	string 							cwd;
	hash_map<string, irc_channel> 	channels;
};
