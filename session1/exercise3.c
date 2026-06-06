#include <stdio.h>

int main(void)
{

    printf("Enter an integer:");
    int input;
    scanf("%d", &input);
    printf("Address :%p, value:%d, size:%lu ", &input, input, sizeof(input));

    int *pointer = &input;
    printf("Address: %p, value: %p, size: %lu\n", &pointer, pointer, sizeof(pointer));
    return 0;

    return 0;
}