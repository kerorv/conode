#include "socketserver.h"

extern "C" Cnode* CreateCnode()
{
	Cnode* socket_node = new SocketServer;
	return socket_node;
}

extern "C" void ReleaseCnode(Cnode* node)
{
	delete node;
}

