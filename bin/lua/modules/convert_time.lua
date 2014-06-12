convert_time = {
	METRICS={
		year=31536000,
		month=2678400,
		week=604800,
		day=86400,
		hour=3600,
		minute=60,
		second=0
	},
	spairs = function(t, order)
		local keys = {}
		for k in pairs(t) do keys[#keys+1] = k end
		
		if order then
			table.sort(keys, function(a,b) return order(t, a, b) end)
		else
			table.sort(keys)
		end
		
		local i = 0
		return function()
			i = i + 1
			if keys[i] then
				return keys[i], t[keys[i]]
			end
		end
	end,
	ago = function(diff)
		local final_value = {};
		-- this must be in order
		for k, v in convert_time.spairs(convert_time.METRICS, function(t,a,b) return t[b] < t[a] end) do
			local total = diff;
			if(v > 0) then
				total = math.floor(diff / v);
				diff = diff % v;
			end
			
			local length = string.format("%d %s", total, k);
			if(total > 0) then
				if(total > 1) then length = length .. "s"; end
				table.insert(final_value, length);
			end
		end
		return table.concat(final_value, ", ");
	end,
	strtotime = function(input, timestamp)
		if(string.len(input)) then
			if(input == "tomorrow") then return timestamp + convert_time.METRICS["day"]; end
			if(input == "yesterday") then return timestamp - convert_time.METRICS["day"]; end
			if(input == "now" or input == "today") then return timestamp; end
			if(string.sub(input, 0, 4) == "last" or string.sub(input, 0, 4) == "next") then
				for k, v in pairs(convert_time.METRICS) do
					if(k == string.sub(input, 6)) then
						if(string.sub(input, 0, 1) == "n") then
							return timestamp + v;
						else
							return timestamp - v;
						end
					end
				end
			else
				for m,n,s in string.gmatch(input, "(.?)(%d+)%s(%S+)") do 
					for k, v in pairs(convert_time.METRICS) do
						if(string.sub(s, 0, string.len(k)) == k) then
							if(m == "+") then
								return (timestamp + (v * n));
							elseif(m == "-") then
								return (timestamp - (v * n));
							end
						end
					end
				end
			end
		end
		return nil;
	end,
	tz_difference = function(csv, one, two)
		-- We're going to have to use the CSV here
	end,
	tz_utc_diff = function(tz)
		--
	end,
	tz_diff = function(first, second)
		-- We need to get the difference between two timezones...
		-- This is way too fucked lol
	end
}

--[[
for id, std_abbr, std_name, dst_abbr, dst_name, gmt_offset, dst_adj, dst_start_rule, start_time, dst_end_rule, end_time in string.gmatch('"Africa/Cairo","EET","EET","EEST","EEST","+02:00:00","+01:00:00","-1;5;4","+00:00:00","-1;5;9","+00:00:00"', '(%S+),(%S+),(%S+),(%S+),(%S+),(%S+),(%S+),(%S+),(%S+),(%S+),(%S+)') do 
print(id .. "," .. std_abbr .. "," .. std_name .. "," .. dst_abbr .. "," .. dst_name .. "," .. gmt_offset .. "," .. dst_adj .. "," .. dst_start_rule .. "," .. start_time .. "," .. dst_end_rule .. "," .. end_time) 
end
]]--

return convert_time;
