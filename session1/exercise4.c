#include <stdio.h>

int main(void)
{
    int n;
    int fac = 1;
    printf("Enter a positive number: ");
    scanf("%d", &n);
    while (n > 0)
    {
        fac *= n;
        n--;
    }
    printf("The factorial is %d", fac);
    return 0;
}
