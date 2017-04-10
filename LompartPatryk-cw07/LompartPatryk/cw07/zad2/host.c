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
#include "main.h"

void sigint_handler(int signum);
void cleanup();

sem_t * sem_id_w;
sem_t * sem_id_r;
int shm_id;
struct shm_mem * shm = (struct shm_mem *)-1;

int main(int argc, char *argv[]) {
    atexit(cleanup);
    struct sigaction act;
    act.sa_handler = sigint_handler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
    srand(time(NULL));

    shm_id = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR);
    if (shm_id < 0 || ftruncate(shm_id, MEM_SIZE) < 0 ||
            (shm = (struct shm_mem *)mmap(0, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) < 0) {
        printf("Error while creating shared memory occurred.\n");
        return 1;
    }
    for (int i = 0; i < ARRAY_LEN; i++) {
        shm->numbers[i] = rand() % MAX_NUM;
    }
    munmap(shm, MEM_SIZE);
    sem_id_w = sem_open(SEM_NAME_W, O_CREAT, S_IWUSR | S_IRUSR, 1);
    sem_id_r = sem_open(SEM_NAME_R, O_CREAT, S_IWUSR | S_IRUSR, MAX_READERS);
    if (sem_id_w < 0 || sem_id_r < 0) {
        printf("Error while creating semaphores occurred.\n");
        return 1;
    }

    printf("Host started.\n");
    while (1)
        pause();
}

void cleanup() {
    if (sem_id_w >= 0) {
        sem_close(sem_id_w);
        sem_unlink(SEM_NAME_W);
    }
    if (sem_id_r >= 0) {
        sem_close(sem_id_r);
        sem_unlink(SEM_NAME_R);
    }
    if (shm_id >= 0) {
        close(shm_id);
        shm_unlink(SHM_NAME);
    }
}

void sigint_handler(int signum) {
    printf("Host closed.\n");
    exit(0);
}