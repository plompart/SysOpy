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
#include "main.h"

void sigint_handler(int signum);
void cleanup();

int sem_id;
int shm_id;

int main(int argc, char *argv[]) {
    atexit(cleanup);
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);

    shm_id = shmget(SHM_KEY, MEM_SIZE, IPC_CREAT | S_IWUSR | S_IRUSR);
    sem_id = semget(SEM_KEY, 3, IPC_CREAT | S_IWUSR | S_IRUSR);

    if (shm_id < 0 || sem_id < 0) {
        printf("Error while creating shared memory and/or semaphores occurred.\n");
        return 1;
    }

    union semun init_sem_val;
    init_sem_val.val = 0;
    semctl(sem_id, 0, SETVAL, init_sem_val);
    init_sem_val.val = 1;
    semctl(sem_id, 2, SETVAL, init_sem_val);
    init_sem_val.val = ARRAY_LEN;
    semctl(sem_id, 1, SETVAL, init_sem_val);

    printf("Host started.\n");
    while (1)
        pause();
}

void cleanup() {
    if (sem_id >= 0)
        semctl(sem_id, 0, IPC_RMID);
    if (shm_id >= 0)
        shmctl(shm_id, IPC_RMID, NULL);
}

void sigint_handler(int signum) {
    printf("Host closed.\n");
    exit(0);
}