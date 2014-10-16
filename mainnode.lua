-- mainnode.lua

require "msgtype"
local core = require "sproto.core"

MainNode = {}
MainNode.__index = MainNode

local function loadpb(pbfile)
	local file = io.open(pbfile, "rb")
	pb = file:read "*a"
	file:close()
	return pb
end

function MainNode.New(nid)
	local node = {}
	setmetatable(node, MainNode)
	node.id = nid
	print("MainNode.New " .. nid)
	-- node.tid = settimer(nid, 1000)
	-- print("settimer id=" .. node.tid)

	local pb = loadpb("proto/client.pb")
	node.sp = core.newproto(pb)
	return node
end

function MainNode:Release()
	print("MainNode:Release()")
end

function MainNode:OnClientMsg(msgtype, msg)
	if (tag == MSG_TYPE_PLAYER_LOGIN) then
		local sptype = core.querytype(self.sp, "loginmsg")
		local loginmsg = core.decode(sptype)
		print("username=" .. loginmsg.username .." password=" .. loginmsg.password)
	end
end

function MainNode:OnMessage(tag, ptr, len)
	print("MainNode:OnMessage: type=" .. tag)
	if (tag == MSG_TYPE_SOCKET_RECVCLIENTMSG) then
		local sptype = core.querytype(self.sp, "SocketRecvClientMsg")
		local clientmsg = core.decode(sptype)
		onclientmsg(clientmsg.msgtype, clientmsg.msg)
	end
end

function MainNode:OnTimer(tid)
	print("MainNode:OnTimer")
end

