#include "main.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "irc.hpp"

#include <boost/regex.hpp>

// We're going to be using a thread now...
void* irc_thread(void* p) {
	irc* pirc = (irc*) p;
	
	char tmpbuf[512];
	
	while(!feof(pirc->sock_fp)) {
		if(!fgets(tmpbuf, sizeof(tmpbuf), pirc->sock_fp)) {
			break;
		}
		
		pirc->buffer = string(tmpbuf);

		if(!pirc->buffer.empty() && pirc->buffer[pirc->buffer.length() - 1] == '\n' && pirc->buffer[pirc->buffer.length() - 2] == '\r') {
			pirc->buffer.pop_back();
			pirc->buffer.pop_back();
		}

		//printf("BUFFER [%s]\n", this->buffer.c_str());
		
		pirc->message_handler();

		memset(tmpbuf, 0, sizeof(tmpbuf));
		
		usleep(1000);
	}
	
	pirc->thread_active = false;
	
	return NULL;
}

void irc::start() {
	srand(std::time(0));

	printf("Starting...\n");

	this->on_start();

	if((this->sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Failed to create socket!\n");
		return;
	}

	memset(&this->sockaddr, 0, sizeof(this->sockaddr));
	this->sockaddr.sin_family = AF_INET;
	hostent* hp = gethostbyname(get_config("irc_serv").c_str());
	bcopy(hp->h_addr, &(this->sockaddr.sin_addr.s_addr), hp->h_length);
	this->sockaddr.sin_port = htons(atoi(get_config("irc_port").c_str()));

	if(connect(this->sock, (struct sockaddr*) &this->sockaddr, sizeof(this->sockaddr)) < 0) {
		printf("Failed to connect to server!\n");
		return;
	}

	this->sock_fp = fdopen(this->sock, "a+");

	if(this->sock_fp == NULL) {
		printf("Failed to open socket stream!\n");
		return;
	}

	this->push("USER " + get_config("irc_uname") + " 127.0.0.1 localhost :" + get_config("irc_uname"));
	this->push("NICK " + get_config("irc_nick"));
	this->identify();

	// Create the thread and start a different loop...
	
	thread_active = true;
	
	pthread_t tid;
	
	if(pthread_create(&tid, NULL, irc_thread, this)) {
		printf("Error creating thread\n");
	}
	
	while(thread_active) {
		this->on_update();
	}
}

void irc::push(string data) {
	string real_data = data + "\r\n";
	send(this->sock, real_data.c_str(), real_data.length(), 0);
}

void irc::set_nick(string nickname) {
	this->push("NICK " + nickname);
}

void irc::identify() {
	if(get_config("irc_pass").empty() == false) {
		this->privmsg("NickServ", "identify " + get_config("irc_pass"));
	}
}

void irc::join_channel(string channel, string key) {
	this->push("JOIN " + channel + ((key.empty()) ? "" : key));
}

void irc::join_channels(vector<string> channels) {
	for(size_t i = 0; i < channels.size(); i++) {
		join_channel(channels[i], "");
	}
}

void irc::privmsg(string to, string msg) {
	this->push("PRIVMSG " + to + " :" + msg);
}

void irc::notice(string to, string msg) {
	this->push("NOTICE " + to + " :" + msg);
}

void irc::version() {
	struct utsname si;
	uname(&si);
	this->push("VERSION HaruhichanBot v3 - " + string(si.sysname));
}

void irc::ghost(string user, string pass) {
	this->privmsg("NickServ", "GHOST " + user + " " + pass);
}

void irc::quit(string msg) {
	this->push("QUIT :" + msg);
}

string irc::bold(string text) {
	return "\2" + text + "\2";
}

string irc::color(string text, string color) {
	return "\x03" + color + text + "\x03";
}

string irc::underline(string text) {
	return "\x1F" + text + "\x1F";
}

bool irc::parse_message() {
	message.clear();
	message.raw = buffer;
	
	boost::regex r1("^[:](\\S+)\\s(\\S+|\\d+)\\s(\\S+)\\s(\\S+)\\s[:](.+)");
	boost::cmatch match;
	
	if(boost::regex_search(buffer.c_str(), match, r1)) {
		message.host = match[1];
		message.hostmask = match[1];
		message.opcode = match[2];
		message.nick = match[3];
		message.user = match[3];
		message.channel = match[4];
		message.args = match[5];
		return true;
	}
	
	boost::regex r2("^(\\S+)\\s[:](\\S+)");
		
	if(boost::regex_search(buffer.c_str(), match, r2)) {
		message.host = get_config("irc_serv");
		message.hostmask = get_config("irc_serv");
		message.opcode = match[1];
		message.args = match[2];
		return true;
	}
	
	boost::regex r3("(:\\S+)\\s(\\S+|\\d+)\\s(\\S+)\\s:(.+)");
			
	if(boost::regex_search(buffer.c_str(), match, r3)) {
		message.hostmask = match[1];
		message.opcode = match[2];
		message.channel = match[3];
		message.args = match[4];
				
		// split hostmask
		unsigned int pos = (message.hostmask.find('!'));
		message.nick = message.hostmask.substr(0, pos);
		message.host = message.hostmask.substr(pos + 1);
				
		// split host
		pos = (message.host.find('@'));
		message.user = message.host.substr(0, pos);
		message.host = message.host.substr(pos + 1);
		return true;
	} 
	
	boost::regex r4("(:|)(\\S+)!(\\S+)@(\\S+)\\s(\\S+)\\s(:|)(\\S+)");
				
	if(boost::regex_search(buffer.c_str(), match, r4)) {
		message.nick = match[2];
		message.user = match[3];
		message.host = match[4];
		message.opcode = match[5];
		message.channel = match[7];
		message.hostmask = match[1]+match[2]+match[3]+"!"+match[4];
		return true;
	}
	
	boost::regex r5("^(?:[:](\\S+) )?(\\S+)(?: (?!:)(.+?))?(?: [:](.+))?$");
					
	if(boost::regex_match(buffer.c_str(), match, r5)) {
		message.preop = match[1];
		message.opcode = match[2];

		for(size_t i = 3; i < match.size(); i++) {
			message.args.append(match[i]);
		}

		return true;
	}
	
	message.error = true;
	
	return false;
}

void irc::message_handler() {
	parse_message();
	
	if(message.error == false) {
		// We will actually have to handle some messages ourselves in this class for basic information purposes.
		// If we want to write things like python bindings, squirrel bindings, etc in the future - we have to handle all IRC related logic in here
		TRACE_MSG;
		if(message.opcode == "PRIVMSG") {
			TRACE_MSG;
			this->on_privmsg(message.channel, message.nick.substr(1), message.user.substr(1), message.args);
			TRACE_MSG;
		} else if(message.opcode == "NOTICE") {
			this->on_notice(message.nick.substr(1), message.channel, message.args);
		} else if(message.opcode == "MODE") {
			this->on_mode(message.channel, message.nick.substr(1), message.args);
		} else if(message.opcode == "JOIN") {
			this->on_join(message.channel, message.nick);
		} else if(message.opcode == "PART") {
			this->on_part(message.channel, message.nick);
		} else if(message.opcode == "NICK") {
			//
		} else if(message.opcode == "PING") {
			this->push("PONG :" + message.args);
		} else if(message.opcode == "001") { // RPL_WELCOME
			// 
		} else if(message.opcode == "004") { // RPL_MYINFO
			//
		} else if(message.opcode == "005") { // RPL_ISUPPORT
			//
		} else if(message.opcode == "042") { // RPL_YOURID
			//
		} else if(message.opcode == "251") { // RPL_LUSERCLIENT
			//
		} else if(message.opcode == "252") { // RPL_LUSEROP
			//
		} else if(message.opcode == "253") { // RPL_LUSERUNKNOWN
			//
		} else if(message.opcode == "254") { // RPL_LUSERCHANNELS
			//
		} else if(message.opcode == "255") { // RPL_LUSERME
			//
		} else if(message.opcode == "265") { // RPL_LOCALUSERS
			//
		} else if(message.opcode == "266") { // RPL_GLOBALUSERS
			//
		} else if(message.opcode == "332") { // RPL_TOPIC
			//
		} else if(message.opcode == "333") { // RPL_TOPICWHOTIME
			//
		} else if(message.opcode == "353") { // RPL_NAMREPLY
			//
		} else if(message.opcode == "366") { // RPL_ENDOFNAMES
			//
		} else if(message.opcode == "372") { // RPL_MOTD
			//
		} else if(message.opcode == "375") { // RPL_MOTDSTART
			//
		} else if(message.opcode == "376") { // RPL_ENDOFMOTD
			//if(config["pass"].empty()) {
				join_channel("#harubot", "");
			//}
		} else if(message.opcode == "433") { // ERR_NICKNAMEINUSE
			if(message.channel == get_config("irc_nick")) {
				printf("Name in use\n");
				set_nick(get_config("irc_altnick"));
				
				// We're going to attempt to ghost them, too
			} else if(message.channel == get_config("irc_altnick")) {
				printf("Unable to register (both nicknames in use!)\n");
				quit("Both names in use! Welp!");
			}
		} else {
			/*
			printf("[%s]:\n[PREOP:%s]\n[HOST:%s]\n[HOSTMASK:%s]\n[NICK:%s]\n[USER:%s]\n[CHANNEL:%s]\n[ARGS:%s]\n",
				message.opcode.c_str(), 	//PRIVMSG
				message.preop.c_str(), 		//
				message.host.c_str(), 		//that.octopus.pillow
				message.hostmask.c_str(), 	//full hostmask
				message.nick.c_str(), 		//:s0beit
				message.user.c_str(), 		//~s0beit
				message.channel.c_str(), 	//harubot
				message.args.c_str()); 		//sup bro
			*/
		}
		
		this->on_message();
	} else {
		printf("Unknown packet format [%s]\n", message.raw.c_str());
	}
}

string irc::random_string(int length) {
	string p;
	for(int i = 0; i < length; i++) {
		int r = rand()%(26+26+10);
		if(r < 26) {
			p += (char)(r + 'a');
		} else if(r < 52) {
			p += (char)(r - 26) + 'A';
		} else {
			p += (char)(r - 52) + '0';
		}
	}
	return p;
}
