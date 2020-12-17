#include <stdio.h>
#include "stdlib.h"
#include "sys/stat.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include<sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEM_AMOUNT 4

#define WRITERS_AMOUNT 3
#define READERS_AMOUNT 5

#define ACT_READER 0
#define ACT_WRITER 1
#define BIN_ACT_WRITER 2
#define WAIT_WRITER 3

#define SEM_ERROR 1
#define SEM_SET_ERR 2
#define SHM_ERR 3
#define MEM_ERR 4
#define FORK_ERR 5
#define SEMOP_ERR 6

struct sembuf startRead[] =
{
    { WAIT_WRITER, 0, 1 },
    { ACT_WRITER,  0, 1 },
    { ACT_READER,  1, 1 }
};

struct sembuf stopRead[] =
{
    { ACT_READER, -1, 1 }
};

struct sembuf startWrite[] =
{
    { WAIT_WRITER,     1, 1 },
    { ACT_READER,      0, 1 },
    { BIN_ACT_WRITER,  -1, 1 },
    { ACT_WRITER,      1, 1 },
    { WAIT_WRITER,     -1, 1 }
};

struct sembuf stopWrite[] =
{
    { ACT_WRITER,     -1, 1 },
    { BIN_ACT_WRITER, 1, 1 }
};

int *sharedMemoryPtr = NULL;

void writer(int semID, int writerID)
{
    if (semop(semID, startWrite, 5) == -1)
    {
        perror("Semop error");
        exit(SEMOP_ERR);
    }

    (*sharedMemoryPtr)++;
    printf("<<---Writer[ID = %d]: write value %d\n", writerID, *sharedMemoryPtr);

    if (semop(semID, stopWrite, 2) == -1)
    {
        perror("Writer semop error");
        exit(SEMOP_ERR);
    }

    sleep(1);
}

void reader(int semID, int readerID)
{
    if (semop(semID, startRead, 3) == -1)
    {
        perror("Semop error");
        exit(SEMOP_ERR);
    }

    printf("->>Reader[ID = %d]: reads value %d\n", readerID, *sharedMemoryPtr);

    if (semop(semID, stopRead, 1) == -1)
    {
        perror("Writer semop error");
        exit(SEMOP_ERR);
    }

    sleep(1);
}

int main()
{
    int semID = semget(IPC_PRIVATE, SEM_AMOUNT, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (semID == -1)
    {
        perror("Semaphore creation error.");
        exit(SEM_ERROR);
    }

    if (semctl(semID, 2, SETVAL, 1) == -1)
    {
        perror("Semaphore set error.");
        exit(SEM_SET_ERR);
    }

    int shmID = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (shmID == -1)
    {
        perror("Shared memory creation error.");
        exit(SHM_ERR);
    }

    sharedMemoryPtr = shmat(shmID, 0, 0);
    if (*sharedMemoryPtr == -1)
    {
        perror("Memory all error.");
        exit(MEM_ERR);
    }

    pid_t childID = -1;
    for (int i = 0; i < WRITERS_AMOUNT; i++)
    {
        if ((childID = fork()) == -1)
        {
            perror("Write fork error");
            exit(FORK_ERR);
        }
        else if (childID == 0)
        {
            for (;;)
                writer(semID, i);
            exit(0);
        }
    }

    for (int i = 0; i < READERS_AMOUNT; i++)
    {
        if ((childID = fork()) == -1)
        {
            perror("Reader fork error");
            exit(FORK_ERR);
        }
        else if (childID == 0)
        {
            for (;;)
                reader(semID, i);
            exit(0);
        }
    }

    int status;
    for (int i = 0; i < WRITERS_AMOUNT + READERS_AMOUNT; i++)
        wait(&status);

    if (shmdt(sharedMemoryPtr) == -1)
    {
        perror("SHMDT error");
        exit(MEM_ERR);
    }

    if (shmctl(shmID, IPC_RMID, NULL) == -1)
    {
        perror("SHMCTL error");
        exit(MEM_ERR);
    }
}
