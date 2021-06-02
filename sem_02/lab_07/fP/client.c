#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SOCKET_NAME "sock.s"
#define BUF_SIZE 256
#define OK 0
#define ANSI_COLOR_RESET   "\x1b[0m"

int main(int argc, char **argv)
{   
    int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Failed to create socket");
        return EXIT_FAILURE;
    }

    struct sockaddr srvr_name;
    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCKET_NAME);

    char *color = colors[atoi(argv[1])];
    for (;;)
    {
        char buf[BUF_SIZE];
        snprintf(buf, BUF_SIZE, "%sMy pid is: %d"ANSI_COLOR_RESET, color, getpid());
        if (sendto(sockfd, buf, strlen(buf), 0, &srvr_name, strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) < 0)
        {
            perror("Failed to send message");
            close(sockfd);
            return EXIT_FAILURE;
        }
        sleep(1);
    }

    return OK;
}