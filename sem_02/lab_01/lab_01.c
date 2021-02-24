// Запускаем два терминала, в одном папка с программой:
// gcc lab_01.c -pthread -Wall -Werror -o main.exe
// sudo ./main.exe
// ps -ajx | grep "./main"

// Во втором терминале открываем cd /var/log
// sudo cat syslog

// По ps: 
// -Ssl
// S - процесс находится в режиме прерываемого сна
// s - процесс является лидером сессии
// l - процесс является многопоточным

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "error.h"
#include "sys/file.h"
#include <pthread.h>
#include "signal.h"

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;

int already_running(void) {
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        syslog(LOG_ERR, "не возможно открыть %s: %s", LOCKFILE,
               strerror(errno));
        exit(1);
    }

    // если файл уже заблокирован завершится неудачей
    if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
        if (errno == EWOULDBLOCK) {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку %s: %s", LOCKFILE,
               strerror(errno));
        exit(1);
    }

    // усекает размер файла до нуля, вдруг предыдущий id демона был длиннее
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return 0;
}

void err_quit(const char *error_message, const char *command) {
    syslog(LOG_ERR, "%s %s", error_message, command);
    exit(1);
}

void *thr_fn(void *arg) {
    int err, signo;

    for (;;) {
        syslog(LOG_INFO, "Поток: %ld", pthread_self());
        err = sigwait(&mask, &signo);
        if (err != 0) {
            syslog(LOG_ERR, "ошибка вызова функции sigwait");
            exit(1);
        }
        switch (signo) {
            case SIGHUP:
                syslog(LOG_INFO, "Вызов getlogin. Результат: %s", getlogin());
                break;
            case SIGTERM:
                syslog(LOG_INFO, "получен сигнал SIGTERM; выход");
                exit(0);
            default:
                syslog(LOG_INFO, "получен непредвиденный сигнал %d\n", signo);
        }
    }
    return 0;
}

void daemonize(const char *cmd) {
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    // Сбросить маску режима создания файла
    // 1 правило, для возможности создания файлов с любыми правами доступа
    umask(0);

    // Получить максимально возможный номер дескриптора файла
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        err_quit("%s: невозможно получить максимальный номер дескриптора ",
                 cmd);

    // Создаем дочерний процесс и завершаем предок
    // 2 правило
    if ((pid = fork()) < 0)
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0) {  // родительский процесс
        exit(0);
    }
    // Создание новой сессии
    // Процесс становится лидером новой сессии, лидером новой группы процессов,
    // теруяет управляющий терминал 3 правило
    setsid();

    // Обеспечить невозможность обретения управляющего терминала в будущем
    // Игнорирование сообщения о потере сигнала с управляющим терминалом
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        err_quit("%s: невозможно игнорировать сигнал SIGHUP ", cmd);

    // Назначить корневой каталог текущим рабочим каталогом,
    // если вдруг он будет запушен с подмонтированной ФС
    // 4 правило
    if (chdir("/") < 0)
        err_quit("%s: невозможно сделать текущим рабочим каталогом /", cmd);

    // Закрыть все открытые фaйловые дескрипторы
    // 5 правило
    if (rl.rlim_max == RLIM_INFINITY) rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++) close(i);

    // Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null
    // Чтобы стандартные функции не влияли на работу демона
    // 6 правило
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    // Инициализировать файл журнала
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1,
               fd2);
        exit(1);
    }
}

int main() {
    char *cmd = "daemon";
    daemonize(cmd);

    if (already_running()) {
        syslog(LOG_ERR, "Демон уже запущен");
        exit(1);
    }

    int err;
	pthread_t tid;
    struct sigaction sa;

    /*
        Восстановить действие по умолчанию для сигнала SIGHUP 
        и заблокировать все сигналы
    */
    sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        err_quit("невозможно восстановить действие SIG_DFL для SIGHUP", "");
        exit(1);   
    }
	sigfillset(&mask);
	if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        err_quit("ошибка выполнения операции SIG_BLOCK", "");
        exit(1);
    }

    /*
     *  Создание потока, который будет заниматься обработкой SIGHYP и SIGTERM
     */
    err = pthread_create(&tid, NULL, thr_fn, 0);
	if (err != 0)
    {
        err_quit("невозможно создать поток", "");
        exit(1);
    }

    while (1) {
        syslog(LOG_INFO, "Идентификатор созданного потока для обработки сигнала: %ld", tid);
        sleep(5);
    }
}