#include <stdio.h>
#include "stdlib.h"
#include "time.h"
#include "sys/stat.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include<sys/shm.h>

#define SEM_AMOUNT 3

#define SEM_BIN 0
#define SEM_E 1
#define SEM_F 2

#define EMPTY_NUM 30

#define SIZE 5

#define SEM_ERROR 1
#define SEM_SET_ERR 2
#define SHM_ERROR 3
#define MEM_ERR 4

int *sharedMemoryPtr = NULL;

int main()
{
    srand(time(NULL));

    int semID = semget(IPC_PRIVATE, SEM_AMOUNT, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
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

    int shmID = shmget(IPC_PRIVATE, SIZE, IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO);
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

}
