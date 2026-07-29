#include <stdio.h>
#include <string.h>

unsigned int mylen(char *s)
{
    unsigned int i;
    for (i = 0; s[i] != '\0'; ++i)
    {
    }
    return i;
}

int main(void)
{

    char str[100];
    printf("Enter a string:");
    fgets(str, 100, stdin);
    printf("Length: %lu %u\n", strlen(str), mylen(str));
    return 0;
}

// we note that we always get length greater by
//  1 unit since the fgets function includes a newline input as well