-- mainnode.lua

local conode = require "conode"
local sproto = require "sproto"
local msgtypes = require "msgtype"

MainNode = {}
MainNode.__index = MainNode

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

local function sendclientmsg(node, sid, type, msg)
	local clientmsg = { sid = sid, msgtype = type, msg = msg }
	local spmsg = node.sp:encode("SocketSendClientMsg", clientmsg)
	conode.sendmsg(100--[[socketnode id]], msgtypes.SOCKET_SENDCLIENTMSG, spmsg)
end

local function onclientmsg(node, sid, msgtype, msg)
	if msgtype == msgtypes.PLAYER_LOGIN then
		local loginmsg = node.sp:decode("LoginMsg", msg)
		assert(loginmsg)
		print("LoginMsg from sid=" .. sid .. 
				" username=" .. loginmsg.username ..
				" password=" .. loginmsg.password)
	elseif msgtype == msgtypes.PLAYER_ECHO then
		local echomsg = node.sp:decode("EchoMsg", msg)
		assert(echomsg)
		print("EchoMsg from sid=" .. sid .. " msg=" .. echomsg.msg)
		local response = { msg = echomsg.msg }
		local respmsg = node.sp:encode("EchoMsg", response)
		sendclientmsg(node, sid, msgtypes.PLAYER_ECHO, respmsg)
	end
end

function MainNode.New(nid)
	local node = {}
	setmetatable(node, MainNode)
	node.id = nid
	print("MainNode.New " .. nid)

	local proto = loadproto("proto/socketnode.sp", "proto/client.sp")
	node.sp = sproto.parse(proto)
	assert(node.sp)
	return node
end

function MainNode:Release()
	print("MainNode:Release()")
end

function MainNode:OnMessage(type, ptr, len)
	if type == msgtypes.SOCKET_RECVCLIENTMSG then
		local clientmsg = self.sp:decodeud("SocketRecvClientMsg", ptr, len)
		print(clientmsg)
		assert(clientmsg)
		onclientmsg(self, clientmsg.sid, clientmsg.msgtype, clientmsg.msg)
	elseif type == msgtypes.SOCKET_CONNECTIONBREAK then
		local connbreakmsg = self.sp:decodeud(
				"SocketConnectionBreakMsg",
				ptr, 
				len)	
		assert(connbreakmsg)
		print("Connection[" .. connbreakmsg.sid 
				.. "] is broken. reason:" 
				.. connbreakmsg.reason)
		local rmconnmsg = { sid = connbreakmsg.sid }
		local spmsg = self.sp:encode("SocketRemoveConnectionMsg", rmconnmsg)
		conode.sendmsg(100--[[socketnode id]], msgtypes.SOCKET_REMOVECONNECTION, spmsg)
	end
end

function MainNode:OnTimer(tid)
	print("MainNode:OnTimer")
end

