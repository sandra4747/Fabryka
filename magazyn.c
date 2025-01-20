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
    int x_empty = 1, y_empty = 1, z_empty = 1;

    // Sprawdzenie sekcji X
    for (int i = 0; i < MAX_SPACE / 6; i += UNIT_SIZE_X) {
        if (shm->magazyn[i] == 'X') {
            x_empty = 0;  // W sekcji X jest przynajmniej jeden komponent
            break;
        }
    }

    // Sprawdzenie sekcji Y
    for (int i = MAX_SPACE / 6; i < MAX_SPACE / 2; i += UNIT_SIZE_Y) {
        if (shm->magazyn[i] == 'Y') {
            y_empty = 0;  // W sekcji Y jest przynajmniej jeden komponent
            break;
        }
    }

    // Sprawdzenie sekcji Z
    for (int i = MAX_SPACE / 2; i < MAX_SPACE; i += UNIT_SIZE_Z) {
        if (shm->magazyn[i] == 'Z') {
            z_empty = 0;  // W sekcji Z jest przynajmniej jeden komponent
            break;
        }
    }

    // Zwróć 1, jeśli którakolwiek sekcja jest całkowicie pusta
    return x_empty || y_empty || z_empty;
}
