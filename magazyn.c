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

int is_magazyn_full(SharedMemory *shm) {
    for (int i = 0; i < MAX_SPACE / 6; i += UNIT_SIZE_X) {
        if (shm->magazyn[i] == '\0') {
            return 0; 
        }
    }

    for (int i = MAX_SPACE / 6; i < MAX_SPACE / 2; i += UNIT_SIZE_Y) {
        if (shm->magazyn[i] == '\0') {
            return 0;  
        }
    }

    for (int i = MAX_SPACE / 2; i < MAX_SPACE; i += UNIT_SIZE_Z) {
        if (shm->magazyn[i] == '\0') {
            return 0;  
        }
    }

    return 1;  // Magazyn pełny (brak wolnych miejsc)
}

int is_any_section_empty(SharedMemory *shm) {
    // Sprawdzenie sekcji X
    for (int i = 0; i < MAX_SPACE / 6; i += UNIT_SIZE_X) {
        if (shm->magazyn[i] == '\0') {  // Jeśli znaleziono puste miejsce w sekcji X
            return 1;  // Sekcja X jest pusta
        }
    }

    // Sprawdzenie sekcji Y
    for (int i = MAX_SPACE / 6; i < MAX_SPACE / 2; i += UNIT_SIZE_Y) {
        if (shm->magazyn[i] == '\0') {  // Jeśli znaleziono puste miejsce w sekcji Y
            return 1;  // Sekcja Y jest pusta
        }
    }

    // Sprawdzenie sekcji Z
    for (int i = MAX_SPACE / 2; i < MAX_SPACE; i += UNIT_SIZE_Z) {
        if (shm->magazyn[i] == '\0') {  // Jeśli znaleziono puste miejsce w sekcji Z
            return 1;  // Sekcja Z jest pusta
        }
    }

    return 0;  // Wszystkie sekcje są pełne
}
