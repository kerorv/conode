ExampleNode = {}
ExampleNode.__index = ExampleNode

function ExampleNode.New(nid)
	-- It will be called after spawnnode
	local node = {}
	setmetatable(node, ExampleNode)
	node.id = nid

	-- Initialize node
	-- TODO
	return node
end

function ExampleNode:Release()
	-- It will be called it after closenode 
end

function ExampleNode:OnMessage(type, ptr, len)
	-- It will be called if this node has message
end

function ExampleNode:OnTimer(tid)
	-- it will be called after settimer
end
