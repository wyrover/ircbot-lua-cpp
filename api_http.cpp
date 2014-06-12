#include "main.hpp"
#include <luabind/object.hpp>
#include <curl/curl.h>
#include "api_http.hpp"
#include "lua_mgr.hpp"

api_http* g_http = new api_http;

struct curlopt_param_t {
	const char* name;
	int def;
	int type;
};

// We want to be able to support HTTP, HTTPS, FTP and such
// So we're leaving in as many options as possible...
// Might allow people to break stuff.
curlopt_param_t g_params[] = {
	{"CURLOPT_HEADER", CURLOPT_HEADER, LUA_TBOOLEAN},
	{"CURLOPT_USERAGENT", CURLOPT_USERAGENT, LUA_TSTRING},
	{"CURLOPT_NOPROGRESS", CURLOPT_NOPROGRESS, LUA_TBOOLEAN},
	{"CURLOPT_NOSIGNAL", CURLOPT_NOSIGNAL, LUA_TBOOLEAN},
	{"CURLOPT_WILDCARDMATCH", CURLOPT_WILDCARDMATCH, LUA_TBOOLEAN},
	{"CURLOPT_FAILONERROR", CURLOPT_FAILONERROR, LUA_TBOOLEAN},
	{"CURLOPT_PROTOCOLS", CURLOPT_PROTOCOLS, LUA_TNUMBER},
	{"CURLOPT_REDIR_PROTOCOLS", CURLOPT_REDIR_PROTOCOLS, LUA_TNUMBER},
	{"CURLOPT_PROXY", CURLOPT_PROXY, LUA_TSTRING},
	{"CURLOPT_PROXYPORT", CURLOPT_PROXYPORT, LUA_TNUMBER},
	{"CURLOPT_PROXYTYPE", CURLOPT_PROXYTYPE, LUA_TNUMBER},
	{"CURLOPT_NOPROXY", CURLOPT_NOPROXY, LUA_TSTRING},
	{"CURLOPT_HTTPPROXYTUNNEL", CURLOPT_HTTPPROXYTUNNEL, LUA_TBOOLEAN},
	{"CURLOPT_SOCKS5_GSSAPI_SERVICE", CURLOPT_SOCKS5_GSSAPI_SERVICE, LUA_TSTRING},
	{"CURLOPT_SOCKS5_GSSAPI_NEC", CURLOPT_SOCKS5_GSSAPI_NEC, LUA_TBOOLEAN},
	{"CURLOPT_INTERFACE", CURLOPT_INTERFACE, LUA_TSTRING},
	{"CURLOPT_LOCALPORT", CURLOPT_LOCALPORT, LUA_TNUMBER},
	{"CURLOPT_LOCALPORTRANGE", CURLOPT_LOCALPORTRANGE, LUA_TNUMBER},
	{"CURLOPT_DNS_CACHE_TIMEOUT", CURLOPT_DNS_CACHE_TIMEOUT, LUA_TNUMBER},
	{"CURLOPT_DNS_USE_GLOBAL_CACHE", CURLOPT_DNS_USE_GLOBAL_CACHE, LUA_TBOOLEAN}, // Obsolete
	{"CURLOPT_BUFFERSIZE", CURLOPT_BUFFERSIZE, LUA_TNUMBER},
	{"CURLOPT_PORT", CURLOPT_PORT, LUA_TNUMBER},
	{"CURLOPT_TCP_NODELAY", CURLOPT_TCP_NODELAY, LUA_TBOOLEAN},
	{"CURLOPT_ADDRESS_SCOPE", CURLOPT_ADDRESS_SCOPE, LUA_TNUMBER},
	{"CURLOPT_TCP_KEEPALIVE", CURLOPT_TCP_KEEPALIVE, LUA_TBOOLEAN},
	{"CURLOPT_TCP_KEEPIDLE", CURLOPT_TCP_KEEPIDLE, LUA_TNUMBER},
	{"CURLOPT_TCP_KEEPINTVL", CURLOPT_TCP_KEEPINTVL, LUA_TNUMBER},
	{"CURLOPT_NETRC", CURLOPT_NETRC, LUA_TNUMBER}, // Param requires defines
	{"CURLOPT_NETRC_FILE", CURLOPT_NETRC_FILE, LUA_TSTRING},
	{"CURLOPT_USERPWD", CURLOPT_USERPWD, LUA_TSTRING},
	{"CURLOPT_PROXYUSERPWD", CURLOPT_PROXYUSERPWD, LUA_TSTRING},
	{"CURLOPT_USERNAME", CURLOPT_USERNAME, LUA_TSTRING},
	{"CURLOPT_PASSWORD", CURLOPT_PASSWORD, LUA_TSTRING},
	{"CURLOPT_PROXYUSERNAME", CURLOPT_PROXYUSERNAME, LUA_TSTRING},
	{"CURLOPT_PROXYPASSWORD", CURLOPT_PASSWORD, LUA_TSTRING},
	{"CURLOPT_HTTPAUTH", CURLOPT_HTTPAUTH, LUA_TNUMBER}, // Param requires defines
	{"CURLOPT_TLSAUTH_TYPE", CURLOPT_TLSAUTH_TYPE, LUA_TNUMBER}, // Param requires defines
	{"CURLOPT_TLSAUTH_USERNAME", CURLOPT_TLSAUTH_USERNAME, LUA_TSTRING},
	{"CURLOPT_TLSAUTH_PASSWORD", CURLOPT_TLSAUTH_PASSWORD, LUA_TSTRING},
	{"CURLOPT_PROXYAUTH", CURLOPT_PROXYAUTH, LUA_TNUMBER},
	{"CURLOPT_SASL_IR", CURLOPT_SASL_IR, LUA_TBOOLEAN},
	{"CURLOPT_BEARER", CURLOPT_BEARER, LUA_TSTRING},
	{"CURLOPT_AUTOREFERER", CURLOPT_AUTOREFERER, LUA_TBOOLEAN},
	{"CURLOPT_ACCEPT_ENCODING", CURLOPT_ACCEPT_ENCODING, LUA_TSTRING},
	{"CURLOPT_TRANSFER_ENCODING", CURLOPT_TRANSFER_ENCODING, LUA_TBOOLEAN},
	{"CURLOPT_FOLLOWLOCATION", CURLOPT_FOLLOWLOCATION, LUA_TBOOLEAN},
	{"CURLOPT_UNRESTRICTED_AUTH", CURLOPT_UNRESTRICTED_AUTH, LUA_TBOOLEAN},
	{"CURLOPT_MAXREDIRS", CURLOPT_MAXREDIRS, LUA_TNUMBER},
	{"CURLOPT_POSTREDIR", CURLOPT_POSTREDIR, LUA_TNUMBER},
};

char g_errorBuffer[CURL_ERROR_SIZE] = {0};

static size_t http_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
	if(userp) {
		((stringstream*)userp)->write((char*)contents, size * nmemb);
	}
	
	return size * nmemb;
}

static void set_options(CURL* ch, luabind::object options) {
	if(luabind::type(options) == LUA_TTABLE) {
		for(size_t i = 0; i < sizeof(g_params); i++) {
			if(luabind::type(options[g_params[i].name]) == g_params[i].type) {
				switch(g_params[i].type) {
					case LUA_TSTRING:
						curl_easy_setopt(ch, g_params[i].def, luabind::object_cast<string>(options[g_params[i].name]).c_str());
					break;
					case LUA_TNUMBER:
						curl_easy_setopt(ch, g_params[i].def, luabind::object_cast<long>(options[g_params[i].name]));
					break;
					case LUA_TBOOLEAN:
						curl_easy_setopt(ch, g_params[i].def, luabind::object_cast<long>(options[g_params[i].name]));
					break;
					case LUA_TTABLE:
						// Don't know if we need a case for this...
					break;
				}
			}
		}
	} else {
		curl_easy_setopt(ch, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:17.0) Gecko/17.0 Firefox/17.0");
		curl_easy_setopt(ch, CURLOPT_FRESH_CONNECT, 1);
	}
}

luabind::object api_http::query(string url, luabind::object options) {
	memset(g_errorBuffer, 0, sizeof(g_errorBuffer));
	
	CURL* ch = curl_easy_init();
	
	if(ch) {
		set_options(ch, options);
		
		stringstream ss;
		
		curl_easy_setopt(ch, CURLOPT_URL, url.c_str());
		curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, http_write_callback);
		curl_easy_setopt(ch, CURLOPT_WRITEDATA, &ss);
		curl_easy_setopt(ch, CURLOPT_ERRORBUFFER, g_errorBuffer);
		
		CURLcode res = curl_easy_perform(ch);
		
		if(res == CURLE_OK) {
			return luabind::object(lua_mgr::singleton()->state(), ss.str());
		}
		
		curl_easy_cleanup(ch);
	}
	
	return luabind::object(); //nil
}

luabind::object api_http::last_error() {
	if(g_errorBuffer[0] == 0) { return luabind::object(); }
	
	return luabind::object(lua_mgr::singleton()->state(), string(g_errorBuffer));
}

api_http* api_http::singleton() {
	return g_http;
}
