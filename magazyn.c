#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "magazyn.h"

void sem_op(int semid, int semnum, int op) {
    struct sembuf operation = {semnum, op, 0};
    semop(semid, &operation, 1);
}

void cleanup(int semid, int shmid, SharedMemory *shm) {
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}