-- mainnode.lua

require "msgtype"
local sproto = require "sproto"

MainNode = {}
MainNode.__index = MainNode

local function loadpb(...)
	local pbs = {}
	for i, v in ipairs{...} do
		print(v)
		local file = io.open(v, "rb")
		assert(file)
		pb = file:read("*a")
		file:close()
		print("pblen=" .. #pb)
		pbs[i] = pb
	end
	return table.concat(pbs)
end

local function loadproto(...)
	local protos = {}
	for i, v in ipairs{...} do
		local file = io.open(v, "r")
		local proto = file:read("*a")
		file:close()
		protos[i] = proto
	end
	return table.concat(protos, "\n")
end

function MainNode.New(nid)
	local node = {}
	setmetatable(node, MainNode)
	node.id = nid
	print("MainNode.New " .. nid)

--	local pb = loadpb("proto/socketnode.pb", "proto/client.pb")
--	print("pb total len=" .. #pb)
	local proto = loadproto("proto/socketnode.sp", "proto/client.sp")
	print(proto)
	node.sp = sproto.parse(proto)
	assert(node.sp)
	return node
end

function MainNode:Release()
	print("MainNode:Release()")
end

function MainNode:SendClientMsg(sid, msgtype, msg)
	local clientmsg = { sid = sid, msgtype = msgtype, msg = msg }
	local spmsg = self.sp:encode("SocketSendClientMsg", clientmsg)
	sendmsg(100--[[socketnode id]], MSG_TYPE_SOCKET_SENDCLIENTMSG, spmsg)
end

function MainNode:OnClientMsg(sid, msgtype, msg)
	if msgtype == MSG_TYPE_PLAYER_LOGIN then
		local loginmsg = self.sp:decode("LoginMsg", msg)
		assert(loginmsg)
		print("LoginMsg from sid=" .. sid .. 
				" username=" .. loginmsg.username ..
				" password=" .. loginmsg.password)
	elseif msgtype == MSG_TYPE_ECHO then
		local echomsg = self.sp:decode("EchoMsg", msg)
		assert(echomsg)
		print("EchoMsg from sid=" .. sid .. " msg=" .. echomsg.msg)
		local response = { msg = echomsg.msg }
		local respmsg = self.sp:encode("EchoMsg", response)
		self:SendClientMsg(sid, MSG_TYPE_ECHO, respmsg)
	end
end

function MainNode:OnMessage(type, ptr, len)
	if type == MSG_TYPE_SOCKET_RECVCLIENTMSG then
		local clientmsg = self.sp:decodeud("SocketRecvClientMsg", ptr, len)
		assert(clientmsg)
		self:OnClientMsg(clientmsg.sid, clientmsg.msgtype, clientmsg.msg)
	elseif type == MSG_TYPE_SOCKET_CONNECTIONBREAK then
		local connbreakmsg = self.sp:decodeud(
				"SocketConnectionBreakMsg",
				ptr, 
				len)	
		assert(connbreakmsg)
		print("Connection[" .. connbreakmsg.sid .. "] is broken. reason:" .. connbreakmsg.reason)
		local rmconnmsg = { sid = connbreakmsg.sid }
		local spmsg = self.sp:encode("SocketRemoveConnectionMsg", rmconnmsg)
		sendmsg(100--[[socketnode id]], MSG_TYPE_SOCKET_REMOVECONNECTION, spmsg)
	end
end

function MainNode:OnTimer(tid)
	print("MainNode:OnTimer")
end

