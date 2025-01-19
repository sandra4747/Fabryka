#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "monter.h"
#include "magazyn.h"
#include "dyrektor.h"

volatile int flag_m = 1;

void handle_sigusr2(int sig) {
    flag_m = 0;
}

void monter(int semid, SharedMemory *shm, char stanowisko) {
    
    signal(SIGUSR2, handle_sigusr2);

    while (flag_m) {

        usleep(rand() % 1000000 + 900000);  // Opóźnienie dostawy

        sem_op(semid, SEM_MUTEX, -1);  // Zablokowanie dostępu do magazynu

        // Inicjalizowanie zmiennych do sprawdzania, czy podzespoły zostały znalezione
        char found_x = 0, found_y = 0, found_z = 0;

        // Szukaj pierwszego dostępnego podzespołu w sekcji X
        while (shm->x_pickup_addr < &shm->magazyn[MAX_SPACE / 6]) {
            if (*shm->x_pickup_addr == 'X') {
                *shm->x_pickup_addr = '\0';  // Odbiór podzespołu X
                shm->x_pickup_addr += UNIT_SIZE_X;  // Przesunięcie wskaźnika o 1 bajt
                found_x = 1;
                break;
            } else {
                shm->x_pickup_addr += UNIT_SIZE_X;
            }
        }

        if (!found_x) {
            shm->x_pickup_addr = &shm->magazyn[0];  // Resetowanie wskaźnika w sekcji X
        }

        // Szukaj pierwszego dostępnego podzespołu w sekcji Y
        while (shm->y_pickup_addr < &shm->magazyn[MAX_SPACE / 2]) {
            if (*shm->y_pickup_addr == 'Y') {
                *shm->y_pickup_addr = '\0';  // Odbiór podzespołu Y
                shm->y_pickup_addr += UNIT_SIZE_Y;  // Przesunięcie wskaźnika o 2 bajty
                found_y = 1;
                break;
            } else {
                shm->y_pickup_addr += UNIT_SIZE_Y;
            }
        }

        if (!found_y) {
            shm->y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Resetowanie wskaźnika w sekcji Y
        }

        // Szukaj pierwszego dostępnego podzespołu w sekcji Z
        while (shm->z_pickup_addr < &shm->magazyn[MAX_SPACE]) {
            if (*shm->z_pickup_addr == 'Z') {
                *shm->z_pickup_addr = '\0';  // Odbiór podzespołu Z
                shm->z_pickup_addr += UNIT_SIZE_Z;  // Przesunięcie wskaźnika o 3 bajty
                found_z = 1;
                break;
            } else {
                shm->z_pickup_addr += UNIT_SIZE_Z;
            }
        }

        if (!found_z) {
            shm->z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];  // Resetowanie wskaźnika w sekcji Z
        }

        // Jeśli wszystkie podzespoły zostały znalezione, zakończ montaż
        if (found_x && found_y && found_z) {
            printf("Monter %c: Zmontowano jeden produkt.\n", stanowisko);
        }

        sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu

        if (is_magazyn_empty(shm)) {  
            if (semctl(semid, SEM_DELIVERY_DONE, GETVAL) == 0) {
                printf("\033[32mKończę pracę! Brak komponentów w magazynie!\033[0m\n");
                exit(0);
            }
        }
        
    }
        sem_op(semid, SEM_MONTER_DONE, -1);

}
