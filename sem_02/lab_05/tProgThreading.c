#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

void *threadFunction(void *file)
{
    for (char curLetter = 'a'; curLetter < '{'; curLetter += 2)
        fprintf(file, "%c", curLetter);
}

int main()
{
    struct stat info;

    FILE *f1 = fopen("out.txt", "w");
    stat("out.txt", &info);
    fprintf(stdout, "first fopen outt.txt in main thread: inode is %ld, filesize is %ld, blksize is %ld\n", info.st_ino, info.st_size, info.st_blksize);

    FILE *f2 = fopen("out.txt", "w");
    stat("out.txt", &info);
    fprintf(stdout, "second fopen outt.txt in main thread: inode is %ld, filesize is %ld, blksize is %ld\n", info.st_ino, info.st_size, info.st_blksize);

    pthread_t thread;
    pthread_create(&thread, NULL, threadFunction, f1);

    for (char curLetter = 'b'; curLetter < '{'; curLetter += 2)
        fprintf(f2, "%c", curLetter);

    pthread_join(thread, NULL);

    fclose(f1);
    stat("out.txt", &info);
fprintf(stdout, "first fopen outt.txt in main thread: inode is %ld, filesize is %ld, blksize is %ld\n", info.st_ino, info.st_size, info.st_blksize);    fclose(f2);
    stat("out.txt", &info);
    fprintf(stdout, "second fopen outt.txt in main thread: inode is %ld, filesize is %ld, blksize is %ld\n", info.st_ino, info.st_size, info.st_blksize);

    return 0;
}
