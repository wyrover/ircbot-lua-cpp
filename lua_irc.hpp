#pragma once

struct lua_irc_script {
	string name;
	string path;
};

struct lua_irc_scripts {
	lua_irc_scripts() : reload_scripts(false) {}
	vector<lua_irc_script*> all;
	bool reload_scripts;
};

class lua_irc : public irc {
public:
	static lua_irc* singleton();
	
	virtual void on_start();
	virtual void on_message();
	virtual void on_privmsg(string channel, string nick, string user, string text);
	virtual void on_notice(string from, string to, string text);
	virtual void on_mode(string channel, string target, string modes);
	virtual void on_join(string channel, string nick);
	virtual void on_part(string channel, string nick);
	virtual void on_nick(string from, string to);
	virtual void on_ping();
	virtual void on_update();
	
	virtual string get_config(string var);
	
	bool reload();
	
	string script_name(size_t i) {
		if(script_num() == 0 || scripts.all[i] == NULL){ 
			return "";
		}
		
		return scripts.all[i]->name;
	}
	
	string script_path(size_t i) {
		if(script_num() == 0 || scripts.all[i] == NULL){ 
			return "";
		}
		
		return scripts.all[i]->path;
	}
	
	size_t script_num() {
		return scripts.all.size();
	}
	
	void set_base_path(string path){ base_path = path; }
	
private:
	lua_irc_scripts scripts;
	string base_path;
};
