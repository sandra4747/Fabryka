#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "magazyn.h"
#include "dostawca.h"

volatile int flag_d = 1; // Flaga działania dostawcy

void handle_sigusr1(int sig) {
    flag_d = 0;
}

void dostawca(int semid, SharedMemory *shm, char type) {
    signal(SIGUSR1, handle_sigusr1); 

    while (flag_d) {
        usleep(rand() % 600000 + 300000); // Opóźnienie (0.3 - 0.9 s)

        sem_op(semid, SEM_MUTEX, -1); // Blokowanie magazynu

        if (type == 'X') {
            if (*shm->x_delivery_addr == '\0') {
                *shm->x_delivery_addr = 'X'; // Dodanie elementu X
                printf("Dostawca X: dostarczono jeden podzespół X.\n");
            }
            shm->x_delivery_addr += UNIT_SIZE_X;
            if (shm->x_delivery_addr >= &shm->magazyn[MAX_SPACE / 6]) {
                shm->x_delivery_addr = &shm->magazyn[0]; // Reset wskaźnika
            }
        } else if (type == 'Y') {
            if (*shm->y_delivery_addr == '\0') {
                *shm->y_delivery_addr = 'Y'; // Dodanie elementu Y
                printf("Dostawca Y: dostarczono jeden podzespół Y.\n");
            }
            shm->y_delivery_addr += UNIT_SIZE_Y;
            if (shm->y_delivery_addr >= &shm->magazyn[MAX_SPACE / 2]) {
                shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6]; // Reset wskaźnika
            }
        } else if (type == 'Z') {
            if (*shm->z_delivery_addr == '\0') {
                *shm->z_delivery_addr = 'Z'; // Dodanie elementu Z
                printf("Dostawca Z: dostarczono jeden podzespół Z.\n");
            }
            shm->z_delivery_addr += UNIT_SIZE_Z;
            if (shm->z_delivery_addr >= &shm->magazyn[MAX_SPACE]) {
                shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2]; // Reset wskaźnika
            }
        }

        sem_op(semid, SEM_MUTEX, 1); // Odblokowanie magazynu

        if (semctl(semid, SEM_MONTER_DONE, GETVAL) == 0) {  
            if (is_magazyn_full(shm)) {
                printf("\033[34mKończę pracę! Brak miejsca w magazynie!\033[0m\n");
                exit(0);
            }
        }
    }

    sem_op(semid, SEM_DELIVERY_DONE, -1); // Informacja o zakończeniu pracy dostawcy
}



