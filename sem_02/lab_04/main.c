#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define BUFFSIZE 10000

// pidof Telegram, для примера
#define PID 1372

void green()
{
    printf("\e[1;92m");
}

void yellow()
{
    printf("\e[1;93m");
}

void cyan()
{
    printf("\e[1;96m");
}

void white()
{
    printf("\e[1;97m");
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

    printf("\nCMDLINE CONTENT (полная командная строка процесса, если он не зомби):\n");
    printf("%s\n", buf);

    fclose(file);
}

void printENVIRON()
{

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/environ", PID);
    FILE *file = fopen(pathToOpen, "r");

    int len;
    char buf[BUFFSIZE];
    printf("\nENVIRON (файл окружения, исходная среда, которая была установлена при запуске выполняемой программы):\n");
    while ((len = fread(buf, 1, BUFFSIZE, file)) > 0)
    {
        for (int i = 0; i < len; i++)
            if (!buf[i])
                buf[i] = '\n';
        buf[len - 1] = '\n';
        printf("%s", buf);
    }

    fclose(file);
}

void printFD()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/fd/", PID);
    DIR *dir = opendir(pathToOpen);

    printf("\nFD(поддиректория, которая содержит одну запись (символическая ссылка на файл) для файла, открытого "
           "процессом):\n");

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
    char *statMeanings[] = {
        "pid - ID процесса",
        "comm - Имя файла",
        "state - Состояние процесса",
        "ppid - ID родительского процесса",
        "pgrp - ID группы процесса",
        "session - ID сессии процесса",
        "tty_nr - управляющий терминал процесса",
        "tpgid - ID внешней группы процессов управляющего терминала",
        "flags - Флаги ядра процесса",
        "minflt - Количество минорных ошибок процесса (Минорные ошибки не включают ошибки загрузки страниц памяти с "
        "диска)",
        "cminflt - Количество минорных ошибок дочерних процессов (Минорные ошибки не включают ошибки загрузки страниц "
        "памяти с диска)",
        "majflt - Количество Мажоных ошибок процесса",
        "cmajflt - Количество Мажоных ошибок дочерних процессов процесса",
        "utime - Количество времени, в течение которого этот процесс исполнялся в пользовательском режиме",
        "stime - Количество времени, в течение которого этот процесс исполнялся в режиме ядра",
        "cutime - Количество времени, в течение которого дети этого процесса исполнялись в "
        "пользовательском режиме",
        "cstime - Количество времени, в течение которого дети этого процесса были запланированы в режиме "
        "ядра",
        "priority - Приоритет процесса",
        "nice - уровень nice процесса, то есть пользовательский приоритет процесса (-20 - высокий, 19 - низкий)",
        "num_threads - Количество потоков",
        "itrealvalue - Время в тиках до следующего SIGALRM отправленного в процесс из-за интервального таймера",
        "starttime - Время, прошедшее с момента загрузки системы, когда процесс был запущен",
        "vsize - Объем виртуальной памяти в байтах",
        "rss - Resident Set (Memory) Size: Количество страниц процесса в физической памяти",
        "rsslim - Текущий предел в байтах для RSS процесса",
        "startcode - Адрес, над которым может работать текст программы",
        "endcode - Адрес, до которого может работать текст программы",
        "startstack - Адрес начала стека",
        "kstkesp - Текущее значение ESP (Stack pointer)",
        "kstkeip - Текущее значение EIP (instruction pointer)",
        "pending - Битовая карта отложенных сигналов, отображаемое в виде десятичного числа",
        "blocked - Битовая карта заблокированных сигналов, отображаемое в виде десятичного числа",
        "sigign - Битовая карта игнорированных сигналов, отображаемое в виде десятичного числа",
        "sigcatch - Битовая карта пойманных сигналов, отображаемое в виде десятичного числа",
        "wchan - Адрес ядрёной функции, где процесс находится в состоянии сна",
        "nswap - Количество страниц, поменявшихся местами",
        "cnswap - Накопительный своп для дочерних процессов",
        "exit_signal - Сигнал, который будет послан родителю, когда процесс будет завершен",
        "processor - Номер процессора, на котором происходило последнее выполнение",
        "rt_priority - Приоритет планирования в реальном времени- число в диапазоне от 1 до 99 для процессов, "
        "запланированных в соответствии с политикой реального времени",
        "policy - Политика планирования",
        "blkio_ticks - Общие блочные задержки ввода/вывода",
        "gtime - Гостевое время процесса",
        "cgtime - Гостевое время дочерних процессов",
        "start_data - Адрес, над которым размещаются инициализированные и неинициализированные данные программы (BSS)",
        "end_data - Адрес, под которым размещаются инициализированные и неинициализированные данные программы (BSS)",
        "start_brk - Адрес, выше которого куча программ может быть расширена с помощью brk",
        "arg_start - Адрес, над которым размещаются аргументы командной строки программы (argv)",
        "arg_end - Адрес, под которым размещаются аргументы командной строки программы (argv)",
        "env_start - Адрес, над которым размещена программная среда",
        "env_end - Адрес, под которым размещена программная среда",
        "exit_code - Состояние выхода потока в форме, сообщаемой waitpid"};

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/stat", PID);
    FILE *file = fopen(pathToOpen, "r");

    char buf[BUFFSIZE];
    fread(buf, 1, BUFFSIZE, file);
    char *token = strtok(buf, " ");

    printf("\nSTAT (информация о состоянии процесса): \n");

    for (int i = 0; token != NULL; i++)
    {
        printf("%3d %s - %s \n", i, statMeanings[i], token);
        token = strtok(NULL, " ");
    }

    fclose(file);
}

void printSTATM()
{
    char *statmMeanings[] = {"size - общее число страниц выделенное процессу в виртуальной памяти",
                             "resident - количество страниц, загруженных в физическую память",
                             "shared - количество общих резедентных страниц",
                             "trs - количество страниц кода",
                             "lrs - количество страниц библиотеки",
                             "drs - количество страниц данных/стека",
                             "dt - dirty pages - данные для записи находятся в кэше страниц, но требуется к "
                             "первоначальной записи на носитель"};

    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/statm", PID);
    FILE *file = fopen(pathToOpen, "r");

    char buf[BUFFSIZE];
    fread(buf, 1, BUFFSIZE, file);
    char *token = strtok(buf, " ");

    printf("\nSTATM (информация об использовании памяти, измеряемой в страницах: \n");

    for (int i = 0; token != NULL; i++)
    {
        printf("%d. %s - %s \n", i, statmMeanings[i], token);
        token = strtok(NULL, " ");
    }

    fclose(file);
}

void printCWD()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/cwd", PID);

    char buf[BUFFSIZE] = {'\0'};

    readlink(pathToOpen, buf, BUFFSIZE);

    printf("CWD (символическая ссылка на директорию процесса):\n");
    printf("%s\n", buf);
}

void printEXE()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/exe", PID);

    char buf[BUFFSIZE] = {'\0'};

    readlink(pathToOpen, buf, BUFFSIZE);

    printf("EXE (символическая ссылка, которая содержит путь к выполненной команде):\n");
    printf("%s\n", buf);
}

void printMAPS()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/maps", PID);

    char buf[BUFFSIZE] = {'\0'};
    FILE *file = fopen(pathToOpen, "r");

    printf("MAPS (список выделенных учатков памяти, используемых процессом и права доступа)\n"
           "Начальный адрес участка памяти : конечный участок участка памяти"
           "\nПрава доступа(r - доступно для чтения, w - доступно для изменения, x - доступно для выполнения, s - "
           "разделяемый, p - "
           "private (copy on write))"
           "\nсмещение распределения"
           "\nстарший и младший номер устройства"
           "\ninode:\n");
    int lengthOfRead;
    while ((lengthOfRead = fread(buf, 1, BUFFSIZE, file)))
    {
        buf[lengthOfRead] = '\0';
        printf("%s\n", buf);
    }

    fclose(file);
}

void printROOT()
{
    char pathToOpen[PATH_MAX];
    snprintf(pathToOpen, PATH_MAX, "/proc/%d/root", PID);

    char buf[BUFFSIZE] = {'\0'};

    readlink(pathToOpen, buf, BUFFSIZE);

    printf("ROOT (символическая ссылка, которая указывает на корень файловой системы, которой принадлежит процесс):\n");
    printf("%s\n", buf);
}

int main()
{
    green();
    printCMDLINE();
    printf("\n");

    yellow();
    printENVIRON();
    printf("\n");

    cyan();
    printFD();
    printf("\n");

    white();
    printSTAT();
    printSTATM();
    printf("\n");

    green();
    printCWD();
    printf("\n");

    yellow();
    printEXE();
    printf("\n");

    cyan();
    printMAPS();
    printf("\n");

    white();
    printROOT();
    printf("\n");

    reset();

    return 0;
}
