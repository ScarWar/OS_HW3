#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>

#define PIPE_READ 0
#define PIPE_WRITE 1
#define PIPE_NAME_PREFIX "/tmp/counter_"


int int_log10(double n) {
    int l = 0;
    while (n >= 1) {
        l++;
        n /= 10;
    }
    return l;
}

int main(int argc, const char **argv) {
    char *filename, *pipe_file_name;
    const char *trgt_c;
    off_t file_offset;
    int length, i, R = 0, fd, pid_size;

    if (argc != 5) {
        printf("sadasd\n");
        return -1;
    }

    trgt_c = argv[1];
    filename = (char *) argv[2];
    file_offset = (off_t) strtol(argv[3], NULL, 10);
    length = strtol(argv[4], NULL, 10);

    fd = open(filename, O_RDONLY, S_IRUSR);
    char *p_void = (char *) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, file_offset);

    // Count number of instances of trgt_c in file
    for (i = 0; i < length; ++i)
        if (*(p_void + i) == *trgt_c)
            R++;


    /* Calculate pipe name */
    pid_t child_pid = getpid();
    pid_size = int_log10(pid_size);
    pipe_file_name = (char *) malloc(strlen(PIPE_NAME_PREFIX) + pid_size + 1);
    pipe_file_name[strlen(PIPE_NAME_PREFIX) + pid_size] = 0;
    sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, child_pid);

    /* Create the pipe */
    if (mkfifo(pipe_file_name, O_RDWR) != 0) {
        // TODO error
        printf("sdsdasdsadasdasd\n");
        return errno;
    }

    /* Send result through pipe */
    int int_size = int_log10(R);
    char *buffer = (char *) malloc((int_size + 1) * sizeof(char));
    if (!buffer) {
        // TODO error message
        printf("asdasds\n");
        return errno;
    }
    buffer[int_size] = 0;
    sprintf(buffer, "%d", R);
    printf("[Debug] - %d %s\n", R, buffer);

    /* Send signal to parent process */

    // kill(getppid(), SIGUSR1);

    /* Write to parent */
    int pipe_fd = open(pipe_file_name, O_WRONLY, S_IWUSR | S_IRUSR);
    write(pipe_fd, buffer, (size_t) int_size + 1); // Write buffer to pipe

    /* Sleep */
    // sleep(1);

    /* Free resources */
    if (munmap(p_void, length) != 0) {
        // TODO error
        return errno;
    }
    close(pipe_fd); // Close pipe write
    unlink(pipe_file_name);
    free(buffer);
    free(pipe_file_name);
    return 0;
}