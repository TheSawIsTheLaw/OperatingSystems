#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd1 = open("alphabet.txt", O_RDONLY);
    int fd2 = open("alphabet.txt", O_RDONLY);

    int firstRead = 1;
    int secondRead = 1;
    char c;
    while (firstRead == 1 || secondRead == 1)
    {
        firstRead = read(fd1, &c, 1);
        if (firstRead == 1)
            write(1, &c, 1);

        secondRead = read(fd2, &c, 1);
        if (secondRead == 1)
            write(1, &c, 1);
    }
    
    return 0;
}
