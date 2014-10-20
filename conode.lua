local conode = {}

-- name: node name
-- return: node id
function conode.spawnnode(name)
	return spawnnode(name)
end

-- id: node id
function conode.closenode(id)
	closenode(id)
end

-- to: target node id
-- type: message type
-- msg: sproto message(can be nil)
function conode.sendmsg(to, type, msg)
	sendmsg(to, type, msg)
end

-- node: instance of node
-- interval: timer interval
-- return: timer id
function conode.settimer(node, interval)
	return settimer(node.id, interval)
end

-- node: instance of node
-- tid: timer id
function conode.killtimer(node, tid)
	killtimer(node.id, tid)
end

return conode

