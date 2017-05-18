#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#define SQ(x) ((x) * (x))
#define abs(x) ((x < 0) ? (-x) : (x))
#define min(x, y) ((x) > (y) ? (x) : (y))
#define PIPE_NAME_PREFIX "/tmp/counter_"


int int_log10(double n) {
    int l = 0;
    while (n >= 1) {
        l++;
        n /= 10;
    }
    return l;
}


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

int get_nforks(long long x) {
    /*  Linear function such that 1K will get 1 fork
        and 4GiB and more will get 16 forks         */
    long long y;
    y = (5 * x) / 1431655424 + 1398096 / 1398101;
    return (int) (1 > min(y, 16) ? 1 : min(y, 16));
}

int main(int argc, const char **argv) {
    int pipe_size, pid_size, ret = -2;
    char *pipe_file_name, *buffer;
    char *execv_argv[6] = {
            "counter.o",
            "s",
            "some.txt",
            "0",
            "64",
            NULL
    };
    pid_t p = fork();
    if (p > 0) {
        sleep(1);
        pid_size = int_log10(p);
        pipe_file_name = (char *) malloc(strlen(PIPE_NAME_PREFIX) + pid_size + 1);
        pipe_file_name[strlen(PIPE_NAME_PREFIX) + pid_size] = 0;
        sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, p);
        int fd = open(pipe_file_name, O_RDONLY, S_IRUSR);
        struct stat sb;
        stat(pipe_file_name, &sb);
        buffer = malloc((size_t) sb.st_size);
        printf("[Debug] - tmp counter size %d\n", (int) sb.st_size);
        if (!buffer) {
            printf("%s\n", "[Info] - malloc error");
        }
        read(fd, buffer, (size_t) sb.st_size);
        printf("%s\n", buffer);
        close(fd);
    } else {
        ret = execv("counter", execv_argv);
    }
    printf("%d\n", ret);


//    s = int_sqrt(213 >> 23);
//    s1 = s;
//    while (s1 > 0) {
//        s1 -= s;
//        fork();
//    }
    return 0;
}