local interval = 30000; -- 30 seconds

local function get_last_check()
	--
end

local function set_last_check()
	--
end

local function cb_releases_start()
	--_G.releases_last_check = 
	
	local fp = io.open("fs/release_last_check");
end

--[[
function file_exists(file)
  local f = io.open(file, "rb")
  if f then f:close() end
  return f ~= nil
end

function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end
]]--

local function cb_releases()
	--
end
