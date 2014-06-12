local function cb_die(channel, nick, user, text)
	if(text == "!die" or text == "!kill") then
		irc:privmsg(channel, "﻿ＴＨＥ  ＰＬＥＡＳＵＲＥ  ＯＦ  ＢＥＩＮＧ  ＣＵＭＭＥＤ  ＩＮＳＩＤＥ");
		irc:quit("Fuck yourself");
	end
end

hook:add("privmsg", "cb_die", cb_die);
