#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "monter.h"
#include "magazyn.h"
#include "dyrektor.h"

volatile int flag_m = 1;

void handle_sigusr2(int sig) {
    flag_m = 0;
}

void monter(int semid, SharedMemory *shm, char stanowisko) {

    signal(SIGUSR2, handle_sigusr2);
    srand(time(NULL) ^ getpid());

    while (flag_m) {

        usleep(rand() % 500000 + 300000);  // Opóźnienie montażu w zakresie od 0.3 do 0.8 sekundy

        sem_op(semid, SEM_MUTEX, -1);  // Zablokowanie dostępu do magazynu

        char found_x = 0, found_y = 0, found_z = 0;
        char *temp_x = NULL, *temp_y = NULL, *temp_z = NULL;

        // Szukanie komponentu X
        while (shm->x_pickup_addr < &shm->magazyn[MAX_SPACE / 6]) {
            if (*shm->x_pickup_addr == 'X') {
                *shm->x_pickup_addr = '\0';  // Odbiór podzespołu X
                temp_x = shm->x_pickup_addr;  // Zapisujemy adres komponentu X
                shm->x_pickup_addr += UNIT_SIZE_X;  
                found_x = 1;
                break;
            } else {
                shm->x_pickup_addr += UNIT_SIZE_X;
            }
        }

        if (!found_x) {
            shm->x_pickup_addr = &shm->magazyn[0];  // Reset wskaźnika w sekcji X
        }

        // Szukanie komponentu Y
        while (shm->y_pickup_addr < &shm->magazyn[MAX_SPACE / 2]) {
            if (*shm->y_pickup_addr == 'Y') {
                *shm->y_pickup_addr = '\0';  // Odbiór podzespołu Y
                temp_y = shm->y_pickup_addr;  // Zapisujemy adres komponentu Y
                shm->y_pickup_addr += UNIT_SIZE_Y;  
                found_y = 1;
                break;
            } else {
                shm->y_pickup_addr += UNIT_SIZE_Y;
            }
        }

        if (!found_y) {
            shm->y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Reset wskaźnika w sekcji Y
        }

        // Szukanie komponentu Z
        while (shm->z_pickup_addr < &shm->magazyn[MAX_SPACE]) {
            if (*shm->z_pickup_addr == 'Z') {
                *shm->z_pickup_addr = '\0';  // Odbiór podzespołu Z
                temp_z = shm->z_pickup_addr;  // Zapisujemy adres komponentu Z
                shm->z_pickup_addr += UNIT_SIZE_Z;  
                found_z = 1;
                break;
            } else {
                shm->z_pickup_addr += UNIT_SIZE_Z;
            }
        }

        if (!found_z) {
            shm->z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];  // Reset wskaźnika w sekcji Z
        }


        // Jeśli znaleziono wszystkie komponenty, zmontowano produkt
        if (found_x && found_y && found_z) {
            printf("Monter %c: Zmontowano jeden produkt.\n", stanowisko);
        } else {
            if (temp_x) {
                *temp_x = 'X';  // Przywrócenie komponentu X
            }
            if (temp_y) {
                *temp_y = 'Y';  // Przywrócenie komponentu Y
            }
            if (temp_z) {
                *temp_z = 'Z';  // Przywrócenie komponentu Z
            }
        }

        sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu

        // Sprawdzanie, czy magazyn jest pusty
        if (semctl(semid, SEM_DELIVERY_DONE, GETVAL) == 0) {  
            if (is_any_section_empty(shm)) {
                printf("\033[32mKończę pracę! Brak komponentów w magazynie!\033[0m\n");
                exit(0);
            }
        }
    }

    // Monter kończy pracę, więc dekrementuje semafor
    sem_op(semid, SEM_MONTER_DONE, -1);
}
