#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFFSIZE 10000

// pidof Telegram, для примера
#define PID 7993

void green()
{
    printf("\033[0;32m");
}

void yellow()
{
    printf("\033[1;33m");
}

void cyan()
{
    printf("\033[0;36m");
}

void purple()
{
    printf("\033[0;35m");
}

void reset()
{
    printf("\033[0m");
}

void printCMDLINE()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/cmdline", PID);
    FILE *file = fopen(pathToOpen, "r");

    char buf[BUFFSIZE];
    int len = fread(buf, 1, BUFFSIZE, file);
    buf[len - 1] = 0;

    printf("pid: %d\ncmdline:%s\n", getpid(), buf);

    fclose(file);
}

void printENVIRON()
{

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/environ", PID);
    FILE *file = fopen(pathToOpen, "r");

    int len;
    char buf[BUFFSIZE];
    while ((len = fread(buf, 1, BUFFSIZE, file)) > 0)
    {
        for (int i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = 10;
        buf[len - 1] = 10;
        printf("%s", buf);
    }

    fclose(file);
}

void printFD()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/fd/", PID);
    DIR *dir = opendir(pathToOpen);

    printf("\nFD RESULTS:\n");

    struct dirent *readDir;
    char string[PATH_MAX];
    char path[BUFFSIZE] = {'\0'};
    while ((readDir = readdir(dir)) != NULL)
    {
        if ((strcmp(readDir->d_name, ".") != 0) && (strcmp(readDir->d_name, "..") != 0))
        {
            sprintf(path, "%s%s", pathToOpen, readDir->d_name);
            readlink(path, string, PATH_MAX);
            printf("{%s} -- %s\n", readDir->d_name, string);
        }
    }

    closedir(dir);
}

void printSTAT()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/stat", PID);
    char buf[BUFFSIZE];

    FILE *file = fopen(pathToOpen, "r");
    fread(buf, 1, BUFFSIZE, file);
    char *tokens = strtok(buf, " ");

    printf("\n STAT RESULTS: \n");

    for (int i = 1; tokens != NULL; i++)
    {
        printf("%d. %s \n", i, tokens);
        tokens = strtok(NULL, " ");
    }

    fclose(file);
}

int main()
{
    green();
    printCMDLINE();
    reset();
    printf("\n");

    yellow();
    printENVIRON();
    reset();
    printf("\n");

    cyan();
    printFD();
    reset();
    printf("\n");

    purple();
    printSTAT();
    reset();

    return 0;
}
