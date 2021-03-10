#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stack.h"

#define RESET "\033[0m"
#define CYAN "\033[0;36m"
#define PURPLE "\033[0;35m"
#define YELLOW "\033[0;33m"
#define RED "\033[0;31m"

#define FTW_F 1 /* файл, не являющийся каталогом */
#define FTW_D 2 /* каталог */

#define ERROR 1
#define SUCCESS 0

typedef int myfunc(const char *, int, int);

static struct stack stk;

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

/*
 * Обойти дерево каталогов, начиная с каталога "pathname".
 * Пользовательская функция func() вызывается для каждого встреченного файла.
 */

void printStats()
{
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (!ntot)
        ntot = 1;

    printf(YELLOW);
    printf("Обычные файлы:                          %-7ld %-5.2f %%\n", nreg, nreg * 100.0 / ntot);
    printf("Каталоги:                               %-7ld %-5.2f %%\n", ndir, ndir * 100.0 / ntot);
    printf("Специальные файлы блочных устройств:    %-7ld %-5.2f %%\n", nblk, nblk * 100.0 / ntot);
    printf("Специальные файлы символьных устройств: %-7ld %-5.2f %%\n", nchr, nchr * 100.0 / ntot);
    printf("FIFO:                                   %-7ld %-5.2f %%\n", nfifo, nfifo * 100.0 / ntot);
    printf("Символические ссылки:                   %-7ld %-5.2f %%\n", nslink, nslink * 100.0 / ntot);
    printf("Сокеты:                                 %-7ld %-5.2f %%\n", nsock, nsock * 100.0 / ntot);
    printf("Всего:                                  %-7ld\n", ntot);
    printf(RESET);
}

void incTypes(struct stat *mode)
{
    switch (mode->st_mode & S_IFMT)
    {
    case S_IFREG:
        nreg++;
        break;
    case S_IFBLK:
        nblk++;
        break;
    case S_IFCHR:
        nchr++;
        break;
    case S_IFIFO:
        nfifo++;
        break;
    case S_IFLNK:
        nslink++;
        break;
    case S_IFSOCK:
        nsock++;
        break;
    }
}

// Обход дерева каталогов
int doPath(myfunc *func, char *fullpath, int depth)
{
    if (depth < 0) // Возврат из просмотренного каталога
    {
        chdir(fullpath);
        return SUCCESS;
    }
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;

    if (lstat(fullpath, &statbuf) == -1) /* Ошибка вызова stat */
        return ERROR;

    if (!S_ISDIR(statbuf.st_mode)) /* Если не каталог */
    {
        incTypes(&statbuf);
        func(fullpath, FTW_F, depth);

        return SUCCESS;
    }

    // Это каталог
    func(fullpath, FTW_D, depth);
    ndir++;

    if ((dp = opendir(fullpath)) == NULL) /* Каталог не доступен */
        return ERROR;

    /* Изменение текущей директории для использования коротких имен */
    if (chdir(fullpath) == -1)
    {
        closedir(dp);
        return ERROR;
    }

    depth++;

    struct stackItem item = {.fileName = "..", .depth = -1};
    push(&stk, &item);

    while ((dirp = readdir(dp)) != NULL)
    {
        /* Пропуск каталогов . и .. */
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
        {
            strcpy(item.fileName, dirp->d_name);
            item.depth = depth;
            push(&stk, &item);
        }
    }

    if (closedir(dp) == -1)
    {
        printf("Невозможно закрыть каталог %s\n", fullpath);
        return ERROR;
    }

    return 0;
}

/* Первый вызов для введенного каталога */
static int myFtw(char *pathname, myfunc *func)
{
    /* Изменение текущей директории для использования коротких имен */
    if (chdir(pathname) == -1)
        return ERROR;

    init(&stk);

    struct stackItem item = {.depth = 0};
    strcpy(item.fileName, pathname);
    push(&stk, &item);

    while (!empty(&stk))
    {
        doPath(func, item.fileName, item.depth);
        item = pop(&stk);
    }
    printStats();

    return 0;
}

static int myFunc(const char *pathname, int type, int depth)
{
    printf(PURPLE);
    for (int i = 0; i < depth; i++)
        printf("    |");

    if (type == FTW_F)
        printf(CYAN);

    printf("    |— %s%s\n", pathname, RESET);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("%sИспользование: %s <начальный_каталог>%s\n", RED, argv[0], RESET);
        return 1;
    }

    return myFtw(argv[1], myFunc); /* выполняет всю работу */
}
