#include <stdio.h>
#include <signal.h>
#include "message.h"
#include "msgrouter.h"
#include "cnode.h"
#include "socketserver.h"

class DummyRouter : public MsgRouter
{
public:
	DummyRouter() {}
	virtual ~DummyRouter() {}

	virtual void SendMsg(const Message& msg)
	{
		printf("Messsage type=%d size=%d content=", msg.type, msg.size);
		for (int i = 0; i < msg.size; ++i)
		{
			printf("%c", msg.content[i]);
		}
		printf("\n");
	}
};

int main(int argc, char* argv[])
{
	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);

	DummyRouter router;
	Cnode* node = new SocketServer;
	if (!node->Create(100, &router, "listenport=8021"))
	{
		printf("socketserver create failed.\n");
		return -1;
	}

	getchar();
	return 0;
}

