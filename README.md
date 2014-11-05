conode
===================

Conode is a concurrent program library that base on message drive model. 

#### Node
Node is a independent unit in conode.It can has id, name, timer, aslo can send or receive message.There are lnode(lua node) and cnode(c/cpp node).
* Lnode

Lnode is a piece of lua code that will be called by library on some event.For example, a lnode named "ExampleNode" like these:
```
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
```
* Cnode

Cnode is a shared library that implement cnode interface.It has own execute routine, and can send or receive message to/from other nodes.Please see include/cnode.h and include/msgrouter.h about detail.

#### conode library
* Conode library provide 6 api for Lnode:
```
spawnnode(name, config)
closenode(id)
cnodeid(name)
sendmsg(to, type, msg)
settimer(interval)
killtimer(timerid)
```
* And provide MsgRouter interface for Cnode(Please see include/msgrouter.h and include/cnode.h).
* open and close library
```
// load config.lua and start conode
// return: conode instance handle
void* conode_start();
void conode_stop(void* handle);
```
If your config.lua like these:
```
worker = 2
mainnode = { name="ExampleNode", config="" }
cnode = {
--	{ name="socketnode", lib="libsocketnode.so", config="listenport=8021" },
--	{ name="dbnode", lib="libdb.so", config="..."}
}
```
Library will initialize two thread, and spawn a node that name is "ExampleNode" from "examplenode.lua" in your running directory.
