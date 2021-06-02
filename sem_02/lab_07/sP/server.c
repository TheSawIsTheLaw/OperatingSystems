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

#define MAX_CLIENTS_COUNT 6

static int sockDescr;
static int clients[MAX_CLIENTS_COUNT];

int cleanup()
{
    close(sockDescr);
    exit(0);
}

void handler(int signum)
{
    cleanup();
    exit(0);
}

void connectionHandler(void)
{
    int socketDescr = accept(sockDescr, NULL, NULL);
    if (socketDescr == -1)
        cleanup();

    for (int i = 0; i < MAX_CLIENTS_COUNT; ++i)
    {
        if (!clients[i])
        {
            clients[i] = socketDescr;
            fprintf(stdout, "%s///Got new connection! We are so popular....///%s \n", "\e[4;32m", "\x1b[0m");
            return;
        }
    }

    perror("Max clients count is reached :( Shutdown...\n");
    cleanup();
}

void clientHandler(int i)
{
    char message[64];
    const ssize_t gotSymbols = recv(clients[i], &message, 64, 0);

    if (!gotSymbols)
    {
        close(clients[i]);
        clients[i] = 0;
        return;
    }

    message[gotSymbols] = '\0';
    fprintf(stdout, "Message was catched: %s\n", message);
}

int main(void)
{
    if ((sockDescr = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Cannot create socket");
        return 1;
    }

    struct sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(SOCKET_PORT);

    if (bind(sockDescr, (struct sockaddr *)&socketAddress, sizeof socketAddress) < 0)
        cleanup();

    if (listen(sockDescr, MAX_CLIENTS_COUNT) < 0)
        cleanup();

    signal(SIGINT, handler);
    fprintf(stdout, "Server is ready to go!\n(Ctrl + C = stop)\n");

    for(;;)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sockDescr, &set);

        int maxSockDescr = sockDescr;

        for (int i = 0; i < MAX_CLIENTS_COUNT; ++i)
        {
            if (clients[i] > 0)
                FD_SET(clients[i], &set);

            if (clients[i] > maxSockDescr)
                maxSockDescr = clients[i];
        }

        if (pselect(maxSockDescr + 1, &set, NULL, NULL, NULL, NULL) < 0)
        {
            cleanup();
            perror("Select failure");
        }

        if (FD_ISSET(sockDescr, &set))
            connectionHandler();

        for (int i = 0; i < MAX_CLIENTS_COUNT; ++i)
            if (clients[i] && FD_ISSET(clients[i], &set))
                clientHandler(i);
    }
}
