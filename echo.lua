-- echo.lua

require "node"

Echo = Node:new(0)

function Echo:oncreate()
	self.tid = 0
	self.tick = 0
	self.to_id = 0
end

function Echo:ondestroy()
--	print("echo[" .. self.id .. "] is destroy.")
end

function Echo:ontimer(tid)
	if (tid == self.tid) then
		message = "tick " .. self.tick .. " " .. self.id;
		self:sendmsg(self.to_id, message)
		self.tick = self.tick + 1
	end
end

function Echo:onmessage(message)
	local tokens = {}
	local idx = 1
	for token in string.gmatch(message, "[^%s]+") do
		tokens[idx] = token
		idx = idx + 1
	end

	if tokens[1] == "oncreate" then
		self:oncreate()
	elseif tokens[1] == "ondestroy" then
		self:ondestroy()
	elseif tokens[1] == "timer" then
		self:ontimer(tonumber(tokens[2]))
	elseif tokens[1] == "start" then
		self.to_id = tonumber(tokens[2])
		interval = tonumber(tokens[3])
		self.tid = self:settimer(interval)
	elseif tokens[1] == "tick" then
		tick = tonumber(tokens[2])
		from_id = tonumber(tokens[3])
		self:sendmsg(from_id, "echo " .. tick)
	elseif tokens[1] == "echo" then
		print("Echo[" .. self.id .. "]:{" .. tonumber(tokens[2]) .. "}")
	end
end

