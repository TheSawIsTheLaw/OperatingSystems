#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define CANT_FORK_ERROR 1
#define SUCCESS 0

 int main(void)
 {
    printf("From Parent. Parent identifiers: parentProcID is %d, groupID is %d\n", getpid(), getpgrp());
    pid_t childpid;
    int children[2];
     
    for (int i = 0; i < 2; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("Can't fork");
            return CANT_FORK_ERROR;
        }
        else if (childpid == 0)
        { 
            sleep(1);
            printf("From child. Child identifiers: childProcID is %d, groupID is %d, parentID is %d\n", getpid(), getpgrp(), getppid());
            return SUCCESS;
        }
        else
            children[i] = childpid;
        
    }

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

    printf("Children IDs from parent proccess: %d and %d\nEnd of parent existence\n\n", children[0], children[1]);

    return SUCCESS;   
 }