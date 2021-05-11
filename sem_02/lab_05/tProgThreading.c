#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *threadFunction(void *file)
{
    for (char curLetter = 'a'; curLetter < '{'; curLetter += 2)
        fprintf(file, "%c", curLetter);
}

int main()
{
    FILE *f1 = fopen("out.txt", "w");
    FILE *f2 = fopen("out.txt", "w");

    pthread_t thread;
    pthread_create(&thread, NULL, threadFunction, f1);

    for (char curLetter = 'b'; curLetter < '{'; curLetter += 2)
        fprintf(f2, "%c", curLetter);

    pthread_join(thread, NULL);

    fclose(f1);
    fclose(f2);

    return 0;
}
