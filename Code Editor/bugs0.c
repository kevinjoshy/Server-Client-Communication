#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE  4096


char* make_copy(int i, char* string)
{
    int len;
    char *s;

    len = strlen(string);
    s   = malloc(len);
    strcpy(s, string);
    if( !(i%1000) )
        printf("i=%d, in make_copy(): %s\n", i, s);
}

int main()
{
    char *ptr, *string = "find the bugs!";

    for(int i=0; i<10000; i++) {
        ptr = malloc(BLOCKSIZE);
        strcpy(ptr, string);
        char *copy = make_copy(i, ptr);
        if( !(i%1000) )
           printf("i=%d, in main(): %s\n", i, copy);
        free(ptr);
    }
}

/ a;
