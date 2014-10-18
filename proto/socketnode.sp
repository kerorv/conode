# socket node message

.SocketSendClientMsg {
	sid 0 : integer
	msgtype 1 : integer 
	msg 2 : string
}

.SocketRecvClientMsg {
	sid 0 : integer
	msgtype 1 : integer
	msg 2 : string
}

.SocketConnectionBreakMsg {
	sid 0 : integer
	reason 1 : integer
}

.SocketKickoffConnectionMsg {
	sid 0 : integer
}

.SocketRemoveConnectionMsg{
	sid 0 : integer
}

