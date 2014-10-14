# socket node message

.SocketSendClientMsg {
	sid 0 : integer
	msg 1 : string
}

.SocketRecvClientMsg {
	sid 0 : integer
	msg 1 : string
}

.SocketConnectionBreakMsg {
	sid 0 : integer
	reason 1 : integer
}

.SocketKickoffConnectionMsg {
	sid 0 : integer
}

