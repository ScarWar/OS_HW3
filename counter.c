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

#define PIPE_NAME_PREFIX "/tmp/counter_"
#define ERROR_HANDLE(s) printf("[Error] - %s, %s\n", s, strerror(errno));

int main(int argc, const char **argv) {
    char *filename, pipe_file_name[strlen(PIPE_NAME_PREFIX) + 20];
    const char *trgt_c;
    off_t file_offset;
    int i, fd;
    long length;
    long long R = 0;

    if (argc != 5) {
        ERROR_HANDLE("Invalid number of arguments");
        return -1;
    }

    trgt_c = argv[1];
    file_offset = strtoll(argv[3], NULL, 10);
    if (errno == ERANGE) {
        ERROR_HANDLE("Failed converting string");
        return errno;
    }
    length = strtol(argv[4], NULL, 10);
    if (errno == ERANGE) {
        ERROR_HANDLE("Failed converting string");
        return errno;
    }

    fd = open(argv[2], O_RDWR | O_CREAT);
    if (fd < 0) {
        ERROR_HANDLE("file open failed");
        return errno;
    }
    // file_offset =  file_offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    // printf("[Debug] - file_offset = %ld\n", file_offset);
    char *map = (char *) mmap(NULL, (size_t) length, PROT_READ, MAP_SHARED, fd, file_offset);
    if (map == MAP_FAILED) {
        ERROR_HANDLE("map failed");
        return errno;
    }

    // Count number of instances of trgt_c in file
    for (i = 0; i < length; ++i)
        if (*(map + i) == *trgt_c)
            R++;


    /* Calculate pipe name */
    sprintf(pipe_file_name, "%s%d", PIPE_NAME_PREFIX, getpid());
    // printf("[Debug] - pipe_file_name = %s\n", pipe_file_name);

    /* Create the pipe */
    if (mkfifo(pipe_file_name, 0644) < 0) {
        ERROR_HANDLE("failed to create named pipe");
        return errno;
    }
    /* Send signal to parent process */
    kill(getppid(), SIGUSR1);


    /* Send result through pipe */
    int pipe_fd = open(pipe_file_name, O_WRONLY);
    if (pipe_fd < 0) {
        ERROR_HANDLE("failed to open pipe");
        return errno;
    }

    /* Write to parent */
    if (write(pipe_fd, &R, sizeof(long long)) != sizeof(long long)) {
        ERROR_HANDLE("failed to write to pipe");
        return errno;
    }
    close(pipe_fd); // Close pipe write

    // printf("[Debug] - number of characters %lld\n", R);

    /* Free resources */
    if (munmap(map, (size_t) length) != 0) {
        ERROR_HANDLE("failed to unmap");
        return errno;
    }
    if (unlink(pipe_file_name) < 0) {
        ERROR_HANDLE("failed to unlink pipe");
        return errno;
    }
    return 0;
}