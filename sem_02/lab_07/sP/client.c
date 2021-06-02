#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <netdb.h>

#define SOCKET_ADDR "localhost"
#define SOCKET_PORT 6666

#define ANSI_COLOR_RESET "\x1b[0m"

int main(int argc, char **argv)
{
    char colors[5][9] = {"\x1b[34m", "\x1b[36m", "\x1b[32m", "\x1b[35m", "\x1b[33m"};

    const int sockDescr = socket(AF_INET, SOCK_STREAM, 0);
    if (sockDescr == -1)
    {
        perror("Cannot create socket");
        return 1;
    }

    struct sockaddr_in addr = {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(SOCKET_PORT)};

    if (connect(sockDescr, (struct sockaddr *)&addr, sizeof addr) < 0)
    {
        perror("Connection try failure");
        return 1;
    }

    char *color = colors[atoi(argv[1])];
    for (;;)
    {
        char msg[64];
        snprintf(msg, 64, "%sHenlo!! Check it: %d" ANSI_COLOR_RESET, color, getpid());
        
        if (sendto(sockDescr, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof addr) < 0)
        {
            perror("Cannot sendto");
            return 1;
        }

        sleep(1);
    }

    return 0;
}
