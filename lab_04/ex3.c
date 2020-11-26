#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define CANT_FORK_ERROR 1
#define CANT_EXECLP_ERROR 2
#define SUCCESS 0

 int main(void)
 {
    printf("From Parent. Parent identifiers: parentProcID is %d, groupID is %d\n", getpid(), getpgrp());
    pid_t childpid;
    int children[2];
    char *commands[2] = {"gcc", "ls"};
    char *arguments[2] = {"--version", "-a"};
     
    for (int i = 0; i < 2; i++)
    {
        if ((childpid = fork()) == -1)
        {
            perror("Can't fork");
            exit(CANT_FORK_ERROR);
        }
        else if (childpid == 0)
        { 
            printf("From child. Child identifiers: childProcID is %d, groupID is %d, parentID is %d\n", getpid(), getpgrp(), getppid());
            
            if (execlp(commands[i], commands[i], arguments[i], NULL) < 0)
                {
                    perror("Can't execlp");
                    exit(CANT_EXECLP_ERROR);
                }

            exit(SUCCESS);
        }
        else
            children[i] = childpid;
        
    }
    printf("\n");

    int childStatus;
    for (int i = 0; i < 2; i++)
    {
        childpid = wait(&childStatus);
        printf("Child has finished: PID = %d; with status: %d\n", childpid, childStatus);
        
        if (WIFEXITED(childStatus))
            printf("Child exited with code %d\n", WEXITSTATUS(childStatus));
        else
            printf("Child terminated abnormally\n");
    }

    printf("Children IDs from parent proccess: %d and %d\nEnd of parent existence\n\n", children[0], children[1]);

    return SUCCESS;   
 }