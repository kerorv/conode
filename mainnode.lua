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

function MainNode:OnClientMsg(msgtype, msg)
	if (msgtype == MSG_TYPE_PLAYER_LOGIN) then
		local loginmsg = self.sp:decode("LoginMsg", msg)
		assert(loginmsg)
		print("LoginMsg username=" .. loginmsg.username .." password=" .. loginmsg.password)
	end
end

function MainNode:OnMessage(tag, ptr, len)
	if (tag == MSG_TYPE_SOCKET_RECVCLIENTMSG) then
		print("MainNode:OnMessage: type=" .. tag .. " len=" .. len)
		local clientmsg = self.sp:decodeud("SocketRecvClientMsg", ptr, len)
		assert(clientmsg)
		self:OnClientMsg(clientmsg.msgtype, clientmsg.msg)
	end
end

function MainNode:OnTimer(tid)
	print("MainNode:OnTimer")
end

