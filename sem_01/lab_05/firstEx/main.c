#include <stdio.h>
#include "stdlib.h"
#include "time.h"
#include "sys/stat.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEM_AMOUNT 3

#define SEM_BIN 0
#define SEM_E 1
#define SEM_F 2

#define EMPTY_NUM 20

#define PRODUCE_NUM 6
#define CONSUME_NUM 6

#define CYCLES 3

#define SEM_ERROR 1
#define SEM_SET_ERR 2
#define SHM_ERROR 3
#define MEM_ERR 4
#define FORK_ERR 5
#define SEMOP_ERR 6

int *sharedMemoryPtr = NULL;
char *sharedCharMemoryPtr = NULL;

char *alphabet = "abcdefghijklmnopqrstuvwxyz";

struct sembuf prodStart[2] =
{
    {SEM_E, -1, 1},
    {SEM_BIN, -1, 1}
};
struct sembuf prodEnd[2] =
{
    {SEM_BIN, 1, 1},
    {SEM_F, 1, 1}
};
struct sembuf readStart[2] =
{
    {SEM_F, -1, 1},
    {SEM_BIN, -1, 1}
};
struct sembuf readEnd[2] =
{
    {SEM_BIN, 1, 1},
    {SEM_E, 1, 1}
};

void producer(int semID, int prodID)
{
    srand(time(NULL));
    sleep(rand() % 4);

    if (semop(semID, prodStart, 2) == -1)
    {
        perror("Producer semop error");
        exit(SEMOP_ERR);
    }

    sharedCharMemoryPtr[sharedMemoryPtr[0]] = alphabet[sharedMemoryPtr[0]];

    printf("<<---Producer[ID = %d]: wrote %c\n", prodID, sharedCharMemoryPtr[sharedMemoryPtr[0]]);
    sharedMemoryPtr[0]++;

    if (semop(semID, prodEnd, 2) == -1)
    {
        perror("Producer semop error");
        exit(SEMOP_ERR);
    }
}

void consumer(int semID, int consID)
{
    srand(time(NULL));
    sleep(rand() % 4);
    if (semop(semID, readStart, 2) == -1)
    {
        perror("Consumer semop error");
        exit(SEMOP_ERR);
    }

    printf("->>Consumer[ID = %d]: read %c\n", consID, sharedCharMemoryPtr[sharedMemoryPtr[1]]);
    sharedMemoryPtr[1]++;

    if (semop(semID, readEnd, 2) == -1)
    {
        perror("Consumer semop error");
        exit(SEMOP_ERR);
    }
}

int main()
{
    int semID = semget(IPC_PRIVATE, SEM_AMOUNT, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (semID == -1)
    {
        perror("Semaphore creation error.");
        exit(SEM_ERROR);
    }

    if (semctl(semID, SEM_BIN, SETVAL, 1) == -1 ||
        semctl(semID, SEM_E, SETVAL, EMPTY_NUM) == -1 ||
        semctl(semID, SEM_F, SETVAL, 0) == -1)
    {
        perror("Semaphore set error.");
        exit(SEM_SET_ERR);
    }

    int shmID = shmget(IPC_PRIVATE, 2 * sizeof(int) + EMPTY_NUM * sizeof(char), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (shmID == -1)
    {
        perror("Shared memory creation error.");
        exit(SHM_ERROR);
    }


    sharedMemoryPtr = shmat(shmID, 0, 0);

    if (*sharedMemoryPtr == -1)
    {
        perror("Memory all error.");
        exit(MEM_ERR);
    }

    sharedCharMemoryPtr = (char *)(sharedMemoryPtr + 2 * sizeof(int));

    pid_t childID = -1;

    for (int i = 0; i < CYCLES; ++i)
    {
        if ((childID = fork()) == -1)
        {
            perror("Producer fork error");
            exit(FORK_ERR);
        }
        else if (childID == 0)
        {
            for (int j = 0; j < PRODUCE_NUM; j++)
                producer(semID, i);
            exit(0);
        }
        if ((childID = fork()) == -1)
        {
            perror("Consumer fork error");
            exit(FORK_ERR);
        }
        else if (childID == 0)
        {
            for (int j = 0; j < CONSUME_NUM; j++)
                consumer(semID, i);
            exit(0);
        }
    }

    int status;
    for (int i = 0; i < CONSUME_NUM + PRODUCE_NUM; i++)
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

    printf("\n\nThis is the end of the task\n");

    exit(0);
}
