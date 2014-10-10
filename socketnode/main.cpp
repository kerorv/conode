#include <stdio.h>
#include <signal.h>
#include "socketserver.h"

int main(int argc, char* argv[])
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	SocketServer server;
	server.Create(8001);
	server.Run();

	return 0;
}

