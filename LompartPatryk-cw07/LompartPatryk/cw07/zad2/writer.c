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
#include "main.h"

void sigint_handler(int signum);
void cleanup();

sem_t * sem_id_w;
sem_t * sem_id_r;
int shm_id;
struct shm_mem * shm = (struct shm_mem *)-1;
struct timespec delay = {0, 100000000l};

int main(int argc, char *argv[]) {
    atexit(cleanup);
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);

    srand(time(NULL));

    shm_id = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_id < 0 || (shm = (struct shm_mem *)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) < 0) {
        printf("Error while opening shared memory occurred.\n");
        return 1;
    }

    sem_id_w = sem_open(SEM_NAME_W, 0);
    sem_id_r = sem_open(SEM_NAME_R, 0);
    if (sem_id_w < 0 || sem_id_r < 0) {
        printf("Error while opening semaphores occurred.\n");
        return 1;
    }

    int start_num;
    int tasks_num;
    int index;
    struct timeval tval;
    while (1) {
        tasks_num = rand() % 100;
        start_num = rand() % MAX_NUM;
        for (int j = 0; j < tasks_num; j++) {
            if (sem_wait(sem_id_w) < 0) {
                printf("Error while waiting for semaphore occurred.\n");
                return 1;
            }

            // wait for all readers to end
            for (int i = 0; i < MAX_READERS; i++) {
                if (sem_wait(sem_id_r) < 0) {
                    printf("Error while waiting for semaphore occurred.\n");
                    return 1;
                }
            }
            index = rand() % ARRAY_LEN;
            shm->numbers[index] = start_num;
            gettimeofday(&tval, NULL);
            printf("%d %ld.%ld Added number %d on position %d. Number of waiting tasks: %d.\n", getpid(), tval.tv_sec, tval.tv_usec/1000, start_num, index, tasks_num - j - 1);
            start_num++;
            for (int i = 0; i < MAX_READERS; i++) {
                if (sem_post(sem_id_r) < 0) {
                    printf("Error while incrementing semaphore occurred.\n");
                    return 1;
                }
            }

            if (sem_post(sem_id_w) < 0) {
                printf("Error while incrementing semaphore occurred.\n");
                return 1;
            }
            nanosleep(&delay, NULL);
        }
        nanosleep(&delay, NULL);
    }
}

void cleanup() {
    if (sem_id_w >= 0) {
        sem_close(sem_id_w);
    }
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
    printf("Writer closed.\n");
    exit(0);
}