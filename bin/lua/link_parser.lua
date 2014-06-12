local function cb_http(channel, nick, user, text)
	if(text == "!http") then
		local r = http:query("http://www.nyaa.se/", nil);
		if(r ~= nil) then
			local ttt = r:match('<meta charset="(%S+)">');
			irc:privmsg(channel, "Title: " .. ttt);
			
			--irc:privmsg(channel, string.sub(r, 0, 220));
		else
			irc:privmsg(channel, "HTTP Request failed ya daft cunt");
		end
	end
end

hook:add("privmsg", "cb_http", cb_http);
