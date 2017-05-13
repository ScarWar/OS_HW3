#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
    int s, s1 = 0;
    int x;
    s = int_sqrt(213 >> 23);
    s1 = s;
    while (s1 > 0) {
        s1 -= s;
        fork();
        char execv_argv[] = {
                "counter",

        };
        execv("counter", execv_argv);
    }
    return 0;
}