#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NAME "sock.s"
#define ANSI_COLOR_RESET "\x1b[0m"

int main(int argc, char **argv)
{
    char colors[5][9] = {"\x1b[34m", "\x1b[36m", "\x1b[32m", "\x1b[35m", "\x1b[33m"};

    int sockDescr;
    if ((sockDescr = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("Cannot create socket");
        return 1;
    }

    struct sockaddr serverName;
    serverName.sa_family = AF_UNIX;
    strcpy(serverName.sa_data, NAME);

    char *color = colors[atoi(argv[1])];
    char buf[64];
    for (;;)
    {
        snprintf(buf, 64, "%sHenlo!! Check it, my id is: %d" ANSI_COLOR_RESET, color, getpid());
        
        if (sendto(sockDescr, buf, strlen(buf), 0, &serverName,
                   strlen(serverName.sa_data) + sizeof(serverName.sa_family)) < 0)
        {
            perror("Failed to send message");
            close(sockDescr);
            return 1;
        }

        sleep(1);
    }

    return 0;
}
