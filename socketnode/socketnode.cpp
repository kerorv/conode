#include "socketserver.h"

Cnode* CreateCnode()
{
	Cnode* socket_node = new SocketServer;
	return socket_node;
}

void ReleaseCnode(Cnode* node)
{
	delete node;
}

