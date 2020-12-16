#include <fcntl.h>
#include <unistd.h>

int main()
{
       char c;    
    // have kernel open two connection to file alphabet.txt
    int fd1 = open("alphabet.txt",O_RDONLY);
    int fd2 = open("alphabet.txt",O_RDONLY);
    // read a char & write it alternatingly from connections fs1 & fd2
    while(read(fd1, &c, 1) == 1 || read(fd2, &c, 1) == 1)
    {
        write(1,&c,1);
        write(1,&c,1);
    }
    return 0;
}