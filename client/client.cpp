#include <stdio.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "cppsproto.h"
#include "loginmsg.h"

struct NetMsg
{
	uint16_t size;
	uint16_t type;
	char content[];
};

#define MSG_TYPE_PLAYER_LOGIN	1001

bool LoadSproto(CppSproto& sp)
{
	std::ifstream ifs("proto/client.pb", std::ifstream::binary);
	if (!ifs)
		return false;
	
	ifs.seekg(0, ifs.end);
	int length = ifs.tellg();
	ifs.seekg(0, ifs.beg);
	std::string pb;
	pb.resize(length, ' ');
		
	char* begin = &*pb.begin();
	ifs.read(begin, length);
	ifs.close();
		
	return sp.Init(pb.data(), pb.size());
}

int main(int argc, char* argv[])
{
	CppSproto sp;
	if (!LoadSproto(sp))
	{
		printf("load sproto fail.\n");
		return -1;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(sockaddr_in));
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	remote_addr.sin_port = htons(8021);
	if (connect(fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) == -1)
	{
		printf("connect fail\n");
		close(fd);
		return -1;
	}

	printf("connect ok\n");

	static char buffer[1024];
	for (;;)
	{
		char cmd;
		scanf("%c", &cmd);
		if (cmd == 'q')
		{
			break;
		}
		else if (cmd == 'r')
		{
			int len = recv(fd, buffer, 1024, 0);
			printf("recv %d bytes:", len);
			for (int i = 0; i < len; ++i)
			{
				printf("%c", buffer[i]);
			}
			printf("\n");
		}
		else if (cmd == 's')
		{
			static char buffer[1024];
			LoginMsg msg;
			msg.SetUsername("abc");
			msg.SetPassword("123");
			int len = sizeof(buffer);
			if (!sp.Encode(&msg, buffer, len))
			{
				printf("encode fail\n");
				continue;
			}
			
			int total_size = sizeof(NetMsg) + len;
			NetMsg* nmsg = (NetMsg*)malloc(total_size);
			nmsg->size = total_size - 2;
			nmsg->type = MSG_TYPE_PLAYER_LOGIN;
			memcpy(nmsg->content, buffer, len);

			int send_bytes = 0;
			for (;;)
			{
				int ret = send(fd, (char*)nmsg, total_size, 0);
				if (ret < 0)
				{
					printf("send fail:%d\n", errno);
					break;
				}
				else
				{
					send_bytes += ret;
					if (send_bytes == total_size)
						break;
				}
			}
			
			// int len = send(fd, "abcde123", 8, 0);
			printf("send %d bytes\n", send_bytes);
			free(nmsg);
		}
	}

	close(fd);
	return 0;
}

