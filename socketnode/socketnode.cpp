#include "socketserver.h"

Cnode* create_cnode()
{
	Cnode* socket_node = new SocketServer;
	return socket_node;
}

void release_cnode(Cnode* node)
{
	delete node;
}

