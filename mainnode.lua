-- mainnode.lua

MainNode = {}
MainNode.__index = MainNode

function MainNode.New(nid)
	local node = {}
	setmetatable(node, MainNode)
	node.id = nid
	print("MainNode.New " .. nid)
	node.tid = settimer(nid, 1000)
	print("settimer id=" .. node.tid)
	return node
end

function MainNode:Release()
	print("MainNode:Release()")
end

function MainNode:OnMessage(tag, ptr, len)
	print("MainNode:OnMessage")
end

function MainNode:OnTimer(tid)
	print("MainNode:OnTimer")
end

