#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc == 3) {
        if (strncmp(argv[1], "A=", 2) != 0) goto bad;
        if (strncmp(argv[2], "B=", 2) != 0) goto bad;
        printf("A say: %s\n", argv[1] + 2);
        printf("B say: %s\n", argv[2] + 2);
    }
    return 0;
bad:
    printf("Something wrong. They don't want to say anything!\n");
    return 1;
}