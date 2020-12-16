#include <stdio.h>

#define OPEN_ERROR 1

int main()
{
    FILE *firstD = fopen("new.txt", "at");
    FILE *secondD = fopen("new.txt", "at");

    if (!firstD || !secondD)
    {
        printf("Error. One of descr was not opened");
        return OPEN_ERROR;
    }

    int queue = 0;
    for (char curChar = 'a'; curChar <= 'z'; curChar++, queue++)
    {
        // fprintf(queue % 2 ? firstD : secondD, &curChar);
        if (queue % 2)
            fprintf(firstD, "%c", curChar);
        else
            fprintf(secondD, "%c", curChar);
    }
}