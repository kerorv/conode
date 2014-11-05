ExampleNode = {}
ExampleNode.__index = ExampleNode

function ExampleNode:Init(id, config)
	-- initialize code
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
