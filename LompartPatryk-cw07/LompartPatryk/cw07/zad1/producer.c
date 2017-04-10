#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "main.h"

void sigint_handler(int signum);
void on_host_closed();
void cleanup();

int sem_id;
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

    shm_id = shmget(SHM_KEY, MEM_SIZE, S_IWUSR);
    sem_id = semget(SEM_KEY, 2, S_IWUSR | S_IRUSR);
    if (shm_id < 0 || sem_id < 0) {
        printf("Error while opening shared memory and/or semaphores occurred.\n");
        return 1;
    }

    shm = shmat(shm_id, NULL, 0);
    if (shm == (void *)-1) {
        printf("Error while accessing shared memory occurred.\n");
        return 1;
    }
    struct sembuf sem_op;
    sem_op.sem_flg = 0;
    int new_task_index;
    int task;
    int tasks_num;
    struct timeval tval;
    while (1) {
        sem_op.sem_num = 1;
        sem_op.sem_op = -1;
        if (semop(sem_id, &sem_op, 1) == -1)
            on_host_closed();

        sem_op.sem_num = 2;
        if (semop(sem_id, &sem_op, 1) == -1)
            on_host_closed();
        tasks_num = semctl(sem_id, 0, GETVAL, NULL);
        new_task_index = (shm->start_index + tasks_num) % ARRAY_LEN;
        sem_op.sem_op = 1;
        if (semop(sem_id, &sem_op, 1) == -1)
            on_host_closed();

        task = rand();
        shm->tasks[new_task_index] = task;
        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        if (semop(sem_id, &sem_op, 1) == -1)
            on_host_closed();
        gettimeofday(&tval, NULL);
        printf("%d %ld.%ld Added number: %d. Number of waiting tasks: %d.\n", getpid(), tval.tv_sec, tval.tv_usec/1000, task, tasks_num + 1);
        nanosleep(&delay, NULL);
    }
}

void on_host_closed() {
    printf("Host closed.\n");
    exit(1);
}

void cleanup() {
    if (shm != (void *)-1)
        shmdt(shm);
}

void sigint_handler(int signum) {
    printf("Producer closed.\n");
    exit(0);
}