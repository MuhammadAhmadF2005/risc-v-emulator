#include <stdio.h>

int main(void)
{
    int number;
    printf("Enter the number you wish to square:");
    scanf("%d", &number);
    int square = number * number;
    printf("The square of %d is %d\n ", number, square);
    return 0;
}