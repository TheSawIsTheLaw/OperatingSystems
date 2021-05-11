#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void *threadFunction(void *gotFd)
{
    int fd = *((int *)gotFd);

    int gotRead = 1;
    char c;
    while (gotRead == 1)
    {
        gotRead = read(fd, &c, 1);
        if (gotRead == 1)
            write(1, &c, 1);
    }
}

int main()
{
    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);

    pthread_t thread;
    pthread_create(&thread, NULL, threadFunction, &fd1);

    int gotRead = 1;
    char c;
    while (gotRead == 1)
    {
        gotRead = read(fd2, &c, 1);
        if (gotRead == 1)
            write(1, &c, 1);
    }

    pthread_join(thread, NULL);
    
    return 0;
}
