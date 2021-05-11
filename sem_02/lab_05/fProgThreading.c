#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>

void *threadFunction(void *gotFs)
{
    int flag = 1;
    char c;
    while (flag == 1)
    {
        flag = fscanf(gotFs, "%c", &c);

        if (flag == 1)
            fprintf(stdout, "\033[34m[additive: %c]\033[0m ", c);
    }
}

int main()
{
    int fd = open("alphabet.txt", O_RDONLY);

    FILE *fs1 = fdopen(fd, "r");
    char buff1[20];
    setvbuf(fs1, buff1, _IOFBF, 20);

    FILE *fs2 = fdopen(fd, "r");
    char buff2[20];
    setvbuf(fs2, buff2, _IOFBF, 20);

    pthread_t additiveThread;
    pthread_create(&additiveThread, NULL, threadFunction, fs1);

    int flag = 1;
    char c;
    while (flag == 1)
    {
        flag = fscanf(fs2, "%c", &c);

        if (flag == 1)
            fprintf(stdout, "\033[32m[main: %c]\033[0m ", c);
    }

    pthread_join(additiveThread, NULL);

    return 0;
}
