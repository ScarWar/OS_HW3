#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
    char *filename;
    const char *trgt_c;
    off_t file_offset;
    int length;

    if (argc != 5) {
        return -1;
    }

    printf("argc = %d\n", argc);
    printf("argv[1] = %s\n", argv[1]);
    printf("argv[2] = %s\n", argv[2]);
    printf("argv[3] = %s\n", argv[3]);
    printf("argv[4] = %s\n", argv[4]);

    trgt_c = argv[1];
    filename = (char *) argv[2];
    file_offset = (off_t) atoi(argv[3]);
    length = atoi(argv[4]);

    printf("Do nothing\nExiting...\n");

    return 0;
}