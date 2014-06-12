local convert_time = require('lua/modules/convert_time');

-- Can even write to files for uptime record

local function cb_uptime_start()
	_G.start_time = os.time();
end

local function cb_uptime(channel, nick, user, text)
	if(text == "!up") then
		irc:privmsg(channel, "Uptime: " .. convert_time.ago(os.time() - _G.start_time));
		irc:privmsg(channel, "TZ: " .. os.date("!%c") .. "[" .. os.time(os.date("!%c")) .. "]");
	end
end

hook:add("start", "cb_uptime_start", cb_uptime_start);
hook:add("privmsg", "cb_uptime", cb_uptime);
