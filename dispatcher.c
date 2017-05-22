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

long total = 0;

int int_log_base(double n, int base) {
    int l = 0;
    while (n >= 1) {
        l++;
        n /= base;
    }
    return l;
}

int get_nforks(int x) {
    /*  Linear function such that 1K will get 1 fork
        and 4GiB and more will get 16 forks         */
    if (x < 2 * getpagesize()) {
        return 1;
    }
    int y = int_log_base(x, 4);
    return (int) max(min(16, y), 1);
}

void my_signal_handler(int signum, siginfo_t *info, void *ptr) {
    long R;
    char pipe_file_name[strlen(PIPE_NAME_PREFIX) + 20];
    printf("Signal sent from process %lu\n", (unsigned long) info->si_pid);
    /* Calculate pipe name */
    pid_t child_pid = info->si_pid;
    sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, child_pid); 
    int pipe_fd = open(pipe_file_name, O_RDONLY, S_IWUSR | S_IRUSR);
    if(pipe_fd < 0){
        // TODO error message
        return;
    }
    ssize_t _n__read = read(pipe_fd, &R, sizeof(long));
    if (_n__read < 0 || _n__read != sizeof(long)) {
        return;
    }
    total += R;
    printf("Signal handler finished\n");
}

int main(int argc, const char **argv) {
    int     pipe_size, pid_size, n_forks, ret = 0;
    char    file_offset[20], chunk_length[20];
    long length;
    // Structure to pass to the registration syscall
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(new_action));
    // Assign pointer to our handler function
    new_action.sa_handler   = my_signal_handler;
    // Setup the flags
    new_action.sa_flags     = SA_SIGINFO;

    /* Calculate length and number of forks */
    struct  stat sb;
    off_t   file_size;
    stat(argv[2], &sb);
    file_size    = sb.st_size;
    n_forks      = get_nforks((long) file_size);
    length       = ((long) file_size) / n_forks;
    long *chunks = (long *) malloc(n_forks);
    if (!chunks) {
        // TODO error message
        printf("Error - memory allocation failed \n");
        return errno;
    }
    int k;
    for (k = 0; k < n_forks - 1; ++k) {
        chunks[k]  = length;
        file_size -= length;
    }
    chunks[k] =  file_size;

    pid_t p;
    off_t offset = 0;
    for (int i = 0; i < n_forks; ++i) {
        offset += length;
        /* Calculate execv_argv */
        sprintf(file_offset, "%ld", offset); // Convert offset to string
        sprintf(chunk_length, "%ld" ,chunks[i]);
        char *execv_argv[6] = {
            "./counter.o",
            (char *) argv[1],
            (char *) argv[2],
            file_offset,
            chunk_length,
            NULL
        };
        sleep(1);
        p = fork();
        if(p == 0)
            ret = execv("counter", execv_argv);
    }
    int status = -1;
    while(wait(&status) < 0);
    printf("Total number of charecters %ld\n", total);
    return 0;
}