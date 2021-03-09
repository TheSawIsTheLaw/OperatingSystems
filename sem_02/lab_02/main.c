#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stack.h"

/* тип функции, которая будет вызываться для каждого встреченного файла */
typedef int myfunc(const char *, int, int);

static myfunc myFunc;
static int myFtw(char *, myfunc *);
static int doPath(myfunc *, char *, int);
static struct stack stk;

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Использование: %s <начальный_каталог>\n", argv[0]);
        return 1;
    }

    return myFtw(argv[1], myFunc); /* выполняет всю работу */
}

/*
 * Обойти дерево каталогов, начиная с каталога "pathname".
 * Пользовательская функция func() вызывается для каждого встреченного файла.
 */
#define FTW_F 1   /* файл, не являющийся каталогом */
#define FTW_D 2   /* каталог */
#define FTW_DNR 3 /* каталог, недоступный для чтения */
#define FTW_NS 4  /* файл, информацию о котором */
/* невозможно получить с помощью stat */

/* Первый вызов для введенного каталога */
static int myFtw(char *pathname, myfunc *func)
{
    /* Изменение текущей директории для использования коротких имен */
    if (chdir(pathname) == -1)
    {
        printf("Ошибка вызова функции chdir %s\n", pathname);
        return 1;
    }

    init(&stk);

    struct stackItem item = {.fileName = ".", .len = 0};
    push(&stk, &item);

    while (!empty(&stk))
    {
        struct stackItem item = pop(&stk);
        doPath(func, item.fileName, item.len);
    }

    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (!ntot)
        ntot = 1;

    printf("regular files: %7ld, %5.2f %%\n", nreg, nreg * 100.0 / ntot);
    printf("directories:   %7ld, %5.2f %%\n", ndir, ndir * 100.0 / ntot);
    printf("block devices: %7ld, %5.2f %%\n", nblk, nblk * 100.0 / ntot);
    printf("char devices:  %7ld, %5.2f %%\n", nchr, nchr * 100.0 / ntot);
    printf("FIFOs:         %7ld, %5.2f %%\n", nfifo, nfifo * 100.0 / ntot);
    printf("symbolic links:%7ld, %5.2f %%\n", nslink, nslink * 100.0 / ntot);
    printf("sockets:       %7ld, %5.2f %%\n", nsock, nsock * 100.0 / ntot);
    printf("Total:         %7ld\n", ntot);

    return 0;
}

/*
 * Обход дерева каталогов, начиная с "fullpath". Если "fullpath" не является
 * каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
 * Для директорий производится рекурсивный вызов функции.
 */
static int doPath(myfunc *func, char *fullpath, int len)
{
    if (len < 0)
    {
        if (chdir(fullpath) == -1)
        {
            printf("Ошибка вызова функции chdir %s\n", fullpath);
            return 1;
        }
        return 0;
    }
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;

    if (lstat(fullpath, &statbuf) == -1) /* Ошибка вызова stat */
        return 1;

    if (S_ISDIR(statbuf.st_mode) == 0) /* Если не каталог */
    {
        switch (statbuf.st_mode & __S_IFMT)
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

        return 1;
    }

    /*
     * Это каталог. Сначала вызовем func(),
     * а затем обработаем все файлы в каталоге.
     */
    ndir++;
    func(fullpath, FTW_D, len);

    if ((dp = opendir(fullpath)) == NULL) /* Каталог не доступен */
        return 1;

    /* Изменение текущей директории для использования коротких имен */
    if (chdir(fullpath) == -1)
    {
        printf("Ошибка вызова функции chdir %s\n", fullpath);
        return 1;
    }

    len += 5;

    struct stackItem item = {.fileName = "..", .len = -1};
    push(&stk, &item);

    while ((dirp = readdir(dp)) != NULL)
    {
        /* Пропуск каталогов . и .. */
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".git") != 0)
        {
            strcpy(item.fileName, dirp->d_name);
            item.len = len;
            push(&stk, &item);
        }
    }

    if (closedir(dp) == -1)
        printf("Невозможно закрыть каталог %s\n", fullpath);

    return 0;
}

static int myFunc(const char *pathname, int type, int len)
{
    if (type == FTW_D)
    {
        for (int i = 0; i < len; i += 5)
        {
            printf("    |");
        }
        printf("    |— %s\n", pathname);
    }

    return 0;
}
