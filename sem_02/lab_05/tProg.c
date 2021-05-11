#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    FILE *f1 = fopen("out.txt", "w");
    FILE *f2 = fopen("out.txt", "w");

    for (char curLetter = 'a'; curLetter < '{'; curLetter++)
        curLetter % 2 ? fprintf(f1, "%c", curLetter) : fprintf(f2, "%c", curLetter);

    fclose(f2);
    fclose(f1);

    return 0;
}
