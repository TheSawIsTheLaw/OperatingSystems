#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define NAME "sock.s"

static int sockDescr;

void makeCleanup()
{
    close(sockDescr);
    unlink(NAME);
}

void handler(int signum)
{
    makeCleanup();
    exit(0);
}

int main(int argc, char **argv)
{
    if ((sockDescr = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("Cannot create socket");
        return 1;
    }

    struct sockaddr addr;
    addr.sa_family = AF_UNIX;
    strcpy(addr.sa_data, NAME);
    if (bind(sockDescr, &addr, strlen(addr.sa_data) + sizeof(addr.sa_family)) < 0)
    {
        perror("Cannot bind socket");
        return 1;
    }

    signal(SIGINT, handler);
    fprintf(stdout, "Server is ready to go.\n(Ctrl + C = stop)\n");

    char gotMessage[64];
    for (;;)
    {
        int numOfSymbols = recv(sockDescr, gotMessage, sizeof(gotMessage), 0);
        if (numOfSymbols <= 0)
        {
            perror("Recv failure");
            makeCleanup();
            return 1;
        }
        gotMessage[numOfSymbols] = '\0';
        
        fprintf(stdout, "Server catched: %s\n", gotMessage);
    }
    
    return 0;
}
