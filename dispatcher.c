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

#define PIPE_NAME_PREFIX "/tmp/counter_"
#define K (1 << 10)
#define M (1 << 20)
#define G (1 << 30)
#define ERROR_HANDLE(s) printf("[Error] - %s, %s\n", s, strerror(errno));

long long total = 0;

int get_nforks(long long x) {
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
    else if (400 * M < x && x <= 700 * M)
        return 13;
    else if (700 * M < x && x <= G)
        return 14;
    else if (G < x && x <= 2 * G)
        return 15;
    return 16;
}

void my_signal_handler(int signum, siginfo_t *info, void *ptr) {
    int pipe_fd;
    long long R;
    char pipe_file_name[strlen(PIPE_NAME_PREFIX) + 20];
    ssize_t _n__read;

    /* Calculate pipe name */
    pid_t child_pid = info->si_pid;
    sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, child_pid);
    pipe_fd = open(pipe_file_name, O_RDONLY, S_IWUSR | S_IRUSR);
    if (pipe_fd < 0) {
        ERROR_HANDLE("open failed");
        return;
    }
    _n__read = read(pipe_fd, &R, sizeof(long long));
    if (_n__read < 0 || _n__read != sizeof(long long)) {
        ERROR_HANDLE("read failed");
        return;
    }
    total += R;
}

int main(int argc, const char **argv) {
    int n_forks, ret = 0, k;
    char file_offset[20], chunk_length[20];
    long long length, *chunks;
    off_t file_size, offset = 0;
    pid_t p;

    if (argc != 3) {
        ERROR_HANDLE("Invalid number of arguments");
        return -1;
    }

    /* Structure to pass to the registration syscall */
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    /* Assign pointer to our handler function */
    new_action.sa_handler = (__sighandler_t) my_signal_handler;
    /* Setup the flags */
    new_action.sa_flags = SA_SIGINFO;

    if (0 != sigaction(SIGUSR1, &new_action, NULL)) {
        ERROR_HANDLE("Signal handle registration failed");
        return -1;
    }

    /* Calculate length and number of forks */
    struct stat sb;
    stat(argv[2], &sb);
    file_size = sb.st_size;
    n_forks = get_nforks(sb.st_size);
    length = (((long long) file_size) / n_forks) & ~(sysconf(_SC_PAGE_SIZE) - 1);
    chunks = (long long *) malloc((size_t) n_forks * sizeof(long long));
    if (!chunks) {
        ERROR_HANDLE("memory allocation failed");
        return errno;
    }

    /* Fill array of read lengths */
    for (k = 0; k < n_forks - 1; ++k) {
        chunks[k]  = length;
        file_size -= length;
    }
    chunks[k] = file_size;

    /* Create forks and execute */
    for (k = 0; k < n_forks; ++k) {
        /* Calculate execv_argv */
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
        sleep(1);
        p = fork();
        if (p < 0) {
            ERROR_HANDLE("fork failed");
            free(chunks);
            return errno;
        }
        if (p == 0) {
            ret = execv("counter", execv_argv);
            if (ret < 0) {
                ERROR_HANDLE("child execution failed");
                free(chunks);
                return errno;
            }
            break;
        }
    }
    free(chunks);

    /* Handle forks */
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
        printf("%lld\n", total);
    else {
        ERROR_HANDLE("Some processes have been corrupted");
        return -1;
    }
    return 0;
}