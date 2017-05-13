#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define SQ(x) ((x) * (x))
#define abs(x) ((x < 0) ? (-x) : (x))
#define min(x, y) ((x) > (y) ? (x) : (y))

int int_sqrt(int n) {
    int upperBound = n, lowerBound = 0;
    int i = 0;
    if (n < 0) return -1;
    if (n == 1) return 1;
    while (upperBound - lowerBound > 1) {
        i = (upperBound + lowerBound) >> 1;
        if (i * i > n)
            upperBound = i;
        else
            lowerBound = i;
    }
    return lowerBound;
}

int main(int argc, const char **argv) {
//    int s, s1 = 0;
    size_t x;
    int ret = -2;
    char *offset;
    char *execv_argv[6] = {
            "counter.o",
            "c",
            "some.txt",
            "8",
            NULL,
            NULL
    };
    for (int i = 0; i < 5; i += 2)
    {
        pid_t p = fork();
        if(p > 0){
            x = (size_t)((ceil(log10( i * 16))+1));
            offset = malloc(x);
            offset[x] = 0;
            sprintf(execv_argv[4],"%d", i * 16);
            printf("%s\n", execv_argv[4]);

            // ret = execv("counter.o", execv_argv);
        } else {
            x = (size_t)((ceil(log10( i * 16 + 8))+1));
            offset = malloc(x);
            offset[x] = 0;
            sprintf(execv_argv[4],"%d", i * 16 + 8);
            printf("%s\n", execv_argv[4]);
            // ret = execv("counter.o", execv_argv);
        }
//        printf("ret = %d\n", ret);
    }

//    s = int_sqrt(213 >> 23);
//    s1 = s;
//    while (s1 > 0) {
//        s1 -= s;
//        fork();
//    }
    return 0;
}