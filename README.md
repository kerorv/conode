conode
===================

Conode is a concurrent program library that base on message drive model. 

#### Node
Node is a independent unit in conode.It has name, id, timer, can send or receive message.There are lnode(lua node) and cnode(c/cpp node).
* Lnode

Lnode is a piece of lua code that will be called by library on some event.For example, a lnode named "ExampleNode" like these:
```
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
```
* Cnode

Cnode is a shared library that implement cnode interface.It has own execute routine, and can send or receive message to/from other nodes.Please see include/cnode.h and include/msgrouter.h about detail.

#### conode library
* Conode library provide 5 api for Lnode:
```
spawnnode(name, config)
closenode(id)
sendmsg(to_id, type, msg)
settimer(id, interval)
killtimer(id, timerid)
```
* And provide MsgRouter interface for Cnode(Please see include/msgrouter.h and include/cnode.h).
* open and close library
```
// worker: concurrent thread count
// mainnode: the name of main node
// config: the config string of node
// return: conode instance handle
void* conode_start(int worker, const char* mainnode, const char* config);
void conode_stop(void* handle);
```
If you call this:
```
conode_start(2, "ExampleNode", NULL)
```
Library will initialize two thread, and spawn a node that name is "ExampleNode" from "examplenode.lua" in your running directory.
