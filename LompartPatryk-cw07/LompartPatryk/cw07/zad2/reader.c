#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/time.h>
#include <string.h>
#include "main.h"

void sigint_handler(int signum);
void cleanup();
int read_args(int argc, char *argv[], int *x, int *only_summary);

sem_t * sem_id_r;
int shm_id;
struct shm_mem * shm = (struct shm_mem *)-1;
struct timespec delay = {0, 100000000l};
int only_summary = 0, total = 0, x;

int main(int argc, char *argv[]) {
    atexit(cleanup);
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);

    char *args_help = "Enter -u x or x where -u means printing only summary, x is a number.\n";
    if (read_args(argc, argv, &x, &only_summary) != 0) {
        printf(args_help);
        return 1;
    }

    srand(time(NULL));

    shm_id = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_id < 0 || (shm = (struct shm_mem *)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) < 0) {
        printf("Error while opening shared memory occurred.\n");
        return 1;
    }

    sem_id_r = sem_open(SEM_NAME_R, 0);
    if (sem_id_r < 0) {
        printf("Error while opening semaphores occurred.\n");
        return 1;
    }

    int counter;
    struct timeval tval;
    while (1) {
        counter = 0;
        if (sem_wait(sem_id_r) < 0) {
            printf("Error while waiting for semaphore occurred.\n");
            return 1;
        }
        for (int i = 0; i < ARRAY_LEN; i++) {
            if (shm->numbers[i] == x) {
                counter++;
            }
        }
        total += counter;
        gettimeofday(&tval, NULL);
        if (!only_summary)
            printf("%d %ld.%ld Found %d times number: %d.\n", getpid(), tval.tv_sec, tval.tv_usec/1000, counter, x);
        if (sem_post(sem_id_r) < 0) {
            printf("Error while incrementing semaphore occurred.\n");
            return 1;
        }
        nanosleep(&delay, NULL);
    }
}

int read_args(int argc, char *argv[], int *x, int *only_summary) {
    if (argc != 2 && argc != 3) {
        printf("Incorrect number of arguments.\n");
        return 1;
    }
    *only_summary = 0;
    if (argc == 3) {
        if (strcmp(argv[1], "-u") == 0) {
            *only_summary = 1;
            *x = atoi(argv[2]);
        }
        else {
            return 1;
        }
    }
    else
        *x = atoi(argv[1]);

    return 0;
}

void cleanup() {
    if (sem_id_r >= 0) {
        sem_close(sem_id_r);
    }
    if (shm_id >= 0) {
        if (shm >= 0) {
            munmap(shm, MEM_SIZE);
        }
        close(shm_id);
    }
}

void sigint_handler(int signum) {
    if (only_summary) {
        printf("%d Found %d times number: %d.\n", getpid(), total, x);
    }
    printf("Reader closed.\n");
    exit(0);
}