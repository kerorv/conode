-- config.lua

worker = 2
cnodes = {
	{ name = "socketnode", id = 100, nodeconfig = { listenport = 8021 } },
	{ name = "dbnode", id = 101, nodeconfig = { host = "127.0.0.1", port = 3306, db = "test", user = "app", password = "123" } }
}
main = "MainNode"

