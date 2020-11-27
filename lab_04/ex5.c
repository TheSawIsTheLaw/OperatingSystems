 #include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CANT_FORK_ERROR 1
#define CANT_EXECLP_ERROR 2
#define CANT_CREATE_PIPE_ERROR 3
#define SUCCESS 0

short canWrite = 0;

void noSIGTSTP()
{
}

void makeCanWriteTrue()
{
    canWrite = 1;
}

 int main(void)
 {
    printf("From Parent. Parent identifiers: parentProcID is %d, groupID is %d\n", getpid(), getpgrp());
    int fd[2];
    pid_t childpid;
    int children[2];
    char *pipeMessages[2] = {"Message №1\n", "Message №2\n"};

    if (pipe(fd) == -1)
    {
        perror("Can't create pipe");
        exit(CANT_CREATE_PIPE_ERROR);
    }

    signal(SIGTSTP, noSIGTSTP);
     
    for (int i = 0; i < 2; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("Can't fork");
            exit(CANT_FORK_ERROR);
        }
        else if (childpid == 0)
        {
            signal(SIGTSTP, makeCanWriteTrue);
            sleep(4);
            if (canWrite)
            {
                close(fd[0]);
                write(fd[1], pipeMessages[i], strlen(pipeMessages[i]));
                printf("Message №%d was sent\n", i + 1);
            }
            else
                printf("Message №%d was NOT sent\n", i + 1);   

            exit(SUCCESS);
        }
        else
            children[i] = childpid;
        
    }
    printf("\n\n");

    int childStatus;
    for (int i = 0; i < 2; i++)
    {
        childpid = wait(&childStatus);
        printf("Child has finished: PID = %d with status: %d\n", childpid, childStatus);
        if (WIFEXITED(childStatus))
            printf("Child exited with code %d\n", WEXITSTATUS(childStatus));
        else if (WIFSIGNALED(childStatus))
            printf("Child process was terminated due to the receipt of a signal that was not caught. Code: %d\n", WTERMSIG(childStatus));
        else if (WIFSTOPPED(childStatus))
            printf("Child process is currently stopped. Code: %d\n", WSTOPSIG(childStatus));
    }

    char gotMessages[32] = { 0 };
    close(fd[1]);
    if (read(fd[0], gotMessages, 32) > 0)
        printf("Received from children:\n%s\n", gotMessages);
    else
        printf("No messages from children.\n");


    printf("Children IDs from parent proccess: %d and %d\nEnd of parent existence\n\n", children[0], children[1]);

    return SUCCESS;   
 }