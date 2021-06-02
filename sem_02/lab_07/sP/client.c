 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 256
#define SOCKET_ADDR "localhost"
#define SOCKET_PORT 9999

#define ANSI_COLOR_RESET   "\x1b[0m"

#define OK 0

int main(int argc, char **argv)
{
    char colors[5][9] = {
    "\x1b[34m",
    "\x1b[36m",
    "\x1b[32m",
    "\x1b[35m",
    "\x1b[33m"
    };
    
	const int master_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_sd == -1) {
		perror("Failed to create socket");
		return EXIT_FAILURE;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(SOCKET_PORT)
	};

	if (connect(master_sd, (struct sockaddr *) &addr, sizeof addr) < 0) {
		perror("Failed to connect");
		return EXIT_FAILURE;
	}

    char *color = colors[atoi(argv[1])];
	for(;;)
    {
		char msg[BUF_SIZE];
		snprintf(msg, BUF_SIZE, "%sMy pid is: %d"ANSI_COLOR_RESET, color, getpid());
		if (sendto(master_sd, msg, strlen(msg), 0, (struct sockaddr *) &addr, sizeof addr) < 0)
        {
			perror("Failed to sendto");
			return EXIT_FAILURE;
		}

		sleep(1);
	}
}
