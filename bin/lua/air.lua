local convert_time = require('lua/modules/convert_time');

local function cb_air(channel, nick, user, text)
	local split = {};
	local p = 0;
	
	for i in string.gmatch(text, "%S+") do
		split[p] = i;
		p = p + 1;
	end

	local command = split[0];
	
	split[0] = nil;
	
	local input = table.concat(split, " ");

	if(command == "!air") then
		local timeval = os.time();
		
		if(string.len(input) ~= 0)then
			timeval = convert_time.strtotime(input, os.time());
		end
		
		if(timeval ~= nil and timeval ~= 0) then
			timeval = timeval + (3600 * 14); -- CST to JST... Will need to be updated when it goes live
		
			local timestr = os.date("%d-%m-%Y", timeval);
			
			irc:notice(nick, "Airing on: " .. timestr .. " (" .. os.date("%c", timeval) .. ")");
			
			local qstring = string.format("SELECT " .. 
				"animedata.name,times.episode,times.time,times.station FROM " .. 
				"animedata, times WHERE `date` = '%s' AND " ..
				"times.animeid = animedata.id ORDER BY `time` ASC", timestr);
			
			if(mysql:connect(irc:config("sql_host"), irc:config("sql_user"), irc:config("sql_pass"), irc:config("sql_db"))) then
				local data = mysql:array_query(qstring);
				
				for k, v in ipairs(data) do
					local timemaker = {};
					timemaker["hour"] = tonumber(string.sub(v.time, 0, 2));
					timemaker["min"] = tonumber(string.sub(v.time, 4, 5));
					timemaker["sec"] = 0;
					timemaker["day"] = tonumber(os.date("%d", timeval));
					timemaker["month"] = tonumber(os.date("%m", timeval));
					timemaker["year"] = tonumber(os.date("%Y", timeval));
					
					local timestmp = os.time(timemaker);
					local timeleft = timestmp - timeval;
					local agotime = "Already Aired";
					
					if(timeleft > 0) then
						agotime = irc:color(irc:bold(convert_time.ago(timeleft)), "3");
					end
					
					-- \2 is bold, shorter than calling the native lol
					irc:notice(nick, 
						string.format("\2%s\2 Episode \2%s\2 at \2%s\2 on \2%s\2 (%s)", 
							v.name, v.episode, v.time, v.station, agotime));
				end
				
				mysql:disconnect();
			else
				irc:notice(nick, "Unable to connect to MySQL!");
			end
		else
			irc:notice(nick, "Invalid date format.");
		end
	end
end

hook:add("privmsg", "cb_air", cb_air);
