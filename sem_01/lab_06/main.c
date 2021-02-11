#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#define WRITERS_SLEEP_IN_MILLI 1000
#define READERS_SLEEP_IN_MILLI 2500

#define WRITERS_AMOUNT 3
#define READERS_AMOUNT 5

#define AMOUNT_OF_ACTIONS 4

#define SUCCESS 0

#define MUTEX_ERR 1
#define EVENT_ERR 2
#define WRITER_CRT_ERR 3
#define READER_CRT_ERR 4

HANDLE mutex, canRead, canWrite;

HANDLE writersThreads[WRITERS_AMOUNT];
HANDLE readersThreads[READERS_AMOUNT];

bool writing = false; // Активный писатель

LONG writersInQueue = 0; // Ждущие писатели
LONG readersInQueue = 0; // Ждущие читатели
LONG readingMembers = 0; // Активные читатели

int sharedMemory = 0;

void startWrite()
{
    InterlockedIncrement(&writersInQueue);

    if (writing || readingMembers > 0)
        WaitForSingleObject(canWrite, INFINITE);

    InterlockedDecrement(&writersInQueue);
    writing = true;
}

void stopWrite()
{
    writing = false;

    if (readersInQueue > 0)
        SetEvent(canRead);
    else
        SetEvent(canWrite);
}

DWORD WINAPI writer(LPVOID lpParams)
{
    for (int i = 0; i < AMOUNT_OF_ACTIONS; i++)
    {
        startWrite();

        sharedMemory++;
        printf("<<---Writer[ID = %d]: write value %d\n", lpParams, sharedMemory);

        stopWrite();
        Sleep(WRITERS_SLEEP_IN_MILLI);
    }

    return SUCCESS;
}

void startRead()
{
    WaitForSingleObject(mutex, INFINITE);

    InterlockedIncrement(&readersInQueue);

    if (writing || writersInQueue > 0)
        WaitForSingleObject(canRead, INFINITE);

    InterlockedDecrement(&readersInQueue);
    InterlockedIncrement(&readingMembers);
    SetEvent(canRead);

    ReleaseMutex(mutex);
}

void stopRead()
{
    InterlockedDecrement(&readingMembers);

    if (readingMembers == 0)
        SetEvent(canWrite);
}

DWORD WINAPI reader(LPVOID lpParams)
{
    while (sharedMemory < WRITERS_AMOUNT * AMOUNT_OF_ACTIONS)
    {
        startRead();

        printf("->>Reader[ID = %d]: read value %d\n", lpParams, sharedMemory);

        stopRead();
        Sleep(READERS_SLEEP_IN_MILLI);
    }

    return SUCCESS;
}

int main()
{
    if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
        perror("Mutex error");
        exit(MUTEX_ERR);
    }

    if ((canRead = CreateEvent(NULL, FALSE, TRUE, NULL)) == NULL)
    {
        perror("CanRead event error");
        exit(EVENT_ERR);
    }

    if ((canWrite = CreateEvent(NULL, FALSE, TRUE, NULL)) == NULL)
    {
        perror("CanWrite event error");
        exit(EVENT_ERR);
    }

    for (int i = 0; i < WRITERS_AMOUNT; i++)
        if ((writersThreads[i] = CreateThread(NULL, 0, writer, i, 0, NULL)) == NULL)
        {
            perror("Writer thread creation error");
            exit(WRITER_CRT_ERR);
        }

    for (int i = 0; i < READERS_AMOUNT; i++)
        if ((readersThreads[i] = CreateThread(NULL, 0, reader, i, 0, NULL)) == NULL)
        {
            perror("Reader thread creation error");
            exit(READER_CRT_ERR);
        }

    WaitForMultipleObjects(WRITERS_AMOUNT, writersThreads, TRUE, INFINITE);
    WaitForMultipleObjects(READERS_AMOUNT, readersThreads, TRUE, INFINITE);

    CloseHandle(mutex);
    CloseHandle(canRead);
    CloseHandle(canWrite);

    return SUCCESS;
}
