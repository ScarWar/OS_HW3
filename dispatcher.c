#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>


#define abs(x) ((x < 0) ? (-x) : (x))
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define PIPE_NAME_PREFIX "/tmp/counter_"
#define K (1 << 10)
#define M (1 << 20)
#define G (1 << 30)
#define ERROR_HANDLE(s) printf("[Error] - %s, %s\n", s, strerror(errno));

long long total = 0;

int int_log_base(double n, int base) {
    int l = 0;
    while (n >= 1) {
        l++;
        n /= base;
    }
    return l;
}

int get_nforks(long long x) {
    /*  Linear function such that 1K will get 1 fork
        and 4GiB and more will get 16 forks         */
    // x /= 2 * getpagesize();
    if (x <= 2 * getpagesize())
        return 1;
    else if (8 * K < x && x <= 50 * K)
        return 2;
    else if (50 * K < x && x <= 250 * K)
        return 3;
    else if (250 * K < x && x <= 800 * K)
        return 4;
    else if (800 * K < x && x <= 1 * M + 600 * K)
        return 5;
    else if (1 * M + 600 * K < x && x <= 3 * M)
        return 6;
    else if (3 * M < x && x <= 20 * M)
        return 7;
    else if (20 * M < x && x <= 50 * M)
        return 8;
    else if (50 * M < x && x <= 100 * M)
        return 9;
    else if (100 * M < x && x <= 200 * M)
        return 10;
    else if (200 * M < x && x <= 300 * M)
        return 11;
    else if (300 * M < x && x <= 400 * M)
        return 12;
    else if (400 * M < x && x <= 500 * M)
        return 13;
    else if (500 * M < x && x <= 600 * M)
        return 14;
    else if (600 * M < x && x <= 700 * M)
        return 15;
    return 16;
}

void my_signal_handler(int signum, siginfo_t *info, void *ptr) {
    long long R;
    char pipe_file_name[strlen(PIPE_NAME_PREFIX) + 20];
    /* Calculate pipe name */
    pid_t child_pid = info->si_pid;
    sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, child_pid);
    int pipe_fd = open(pipe_file_name, O_RDONLY, S_IWUSR | S_IRUSR);
    if(pipe_fd < 0){
        ERROR_HANDLE("open failed");
        return;
    }
    ssize_t _n__read = read(pipe_fd, &R, sizeof(long long));
    if (_n__read < 0 || _n__read != sizeof(long long)) {
        ERROR_HANDLE("read failed");
        return;
    }
    total += R;
    // printf("[Debug] - Signal handler finished\n[Debug] - Number returned %lld\n", R);
}

int main(int argc, const char **argv) {
    int n_forks, ret = -2, k;
    char file_offset[20], chunk_length[20];
    long long length;

    if (argc != 3) {
        ERROR_HANDLE("Invalid number of arguments");
        return -1;
    }

    // Structure to pass to the registration syscall
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    // Assign pointer to our handler function
    new_action.sa_handler = my_signal_handler;
    // Setup the flags
    new_action.sa_flags = SA_SIGINFO;

    if (0 != sigaction(SIGUSR1, &new_action, NULL)) {
        ERROR_HANDLE("Signal handle registration failed");
        return -1;
    }

    /* Calculate length and number of forks */
    struct  stat sb;
    off_t   file_size;
    stat(argv[2], &sb);
    file_size    = sb.st_size;
    n_forks = get_nforks(sb.st_size);
    // printf("[Debug] - number of forks = %d\n", n_forks);
    length = (long long) (((long long) file_size) / n_forks) & ~(sysconf(_SC_PAGE_SIZE) - 1);
    long long *chunks = (long long *) malloc((size_t) n_forks * sizeof(long long));
    // printf("[Debug] - length = %ld\n", length);
    if (!chunks) {
        ERROR_HANDLE("memory allocation failed");
        return errno;
    }
    for (k = 0; k < n_forks - 1; ++k) {
        chunks[k]  = length;
        file_size -= length;
        // printf("[Debug] - chunks[%d] = %ld\n", k, length);
    }
    chunks[k] = file_size;
    // printf("[Debug] - chunks[%d] = %ld\n", k, file_size);
    pid_t p;
    off_t offset = 0;
    for (k = 0; k < n_forks; ++k) {
        /* Calculate execv_argv */
        // printf("[Debug] - new parameters offset - %ld, chunk - %ld \n",offset, chunks[k] );
        sprintf(file_offset, "%lld", (long long) offset); // Convert offset to string
        sprintf(chunk_length, "%lld", chunks[k]);
        offset += length;
        char *execv_argv[6] = {
                "./counter",
                (char *) argv[1],
                (char *) argv[2],
                file_offset,
                chunk_length,
                NULL
        };
        // printf("\n[Debug] - fork #%d\n", k);
        // printf("[Debug] - execv_argv = %s %s %s %s %s %s \n", execv_argv[0], execv_argv[1], execv_argv[2], execv_argv[3], execv_argv[4], execv_argv[5]);
        sleep(1);
        p = fork();
        if (p < 0) {
            ERROR_HANDLE("fork failled");
            return errno;
        }
        if (p == 0) {
            // printf("[Debug] - child pid %d\n", getpid());
            // sleep(1);
            ret = execv("counter", execv_argv);
            // printf("ret = %d", ret);
            break;
        }
    }
    free(chunks);
    int status;
    char flag = 0;
    while (1) {
        p = wait(&status);
        if (p > 0 && WIFEXITED(status) && WEXITSTATUS(status) != 0)
            flag = 1;
        else if (p < 0 && errno == ECHILD)
            break;
    }
    if (!flag)
        printf("Total number of characters = %lld\n", total);
    else {
        ERROR_HANDLE("Some processes have been corrupted");
        return -1;
    }
    return 0;
}