#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "magazyn.h"
#include "dostawca.h"
#include "dyrektor.h"

volatile int flag_d = 1;

void handle_sigusr1(int sig) {
    flag_d = 0;
}

void dostawca(int semid, SharedMemory *shm, char type) {

    signal(SIGUSR1, handle_sigusr1);

    while (flag_d) {

        usleep(rand() % 300000 + 800000);  // Opóźnienie dostawy 

        sem_op(semid, SEM_MUTEX, -1);  // Zablokowanie dostępu do magazynu

        // Szukaj pierwszego wolnego miejsca w odpowiednich sekcjach
        if (type == 'X') {
            // Sprawdź całą sekcję X
            while (shm->x_delivery_addr + UNIT_SIZE_X <= &shm->magazyn[MAX_SPACE / 6]) {
                if (*shm->x_delivery_addr == '\0') {
                    *shm->x_delivery_addr = 'X';  // Zapisz podzespół X
                    shm->x_delivery_addr += UNIT_SIZE_X;  // Przesuń wskaźnik
                    printf("Dostawca X: dostarczono jeden podzespół X.\n");
                    break;
                }
                shm->x_delivery_addr += UNIT_SIZE_X;
            }

            shm->x_delivery_addr = &shm->magazyn[0];  // Resetuj wskaźnik

        }

        else if (type == 'Y') {
            // Sprawdź całą sekcję Y
            while (shm->y_delivery_addr + UNIT_SIZE_Y <= &shm->magazyn[MAX_SPACE / 2]) {
                if (*shm->y_delivery_addr == '\0') {
                    *shm->y_delivery_addr = 'Y';  // Zapisz podzespół Y
                    shm->y_delivery_addr += UNIT_SIZE_Y;  // Przesuń wskaźnik
                    printf("Dostawca Y: dostarczono jeden podzespół Y.\n");
                    break;
                }
                shm->y_delivery_addr += UNIT_SIZE_Y;
            }

            shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];  // Resetuj wskaźnik dla Y

        }

        else if (type == 'Z') {
            // Sprawdź całą sekcję Z
            while (shm->z_delivery_addr + UNIT_SIZE_Z <= &shm->magazyn[MAX_SPACE]) {
                if (*shm->z_delivery_addr == '\0') {
                    *shm->z_delivery_addr = 'Z';  // Zapisz podzespół Z
                    shm->z_delivery_addr += UNIT_SIZE_Z;  // Przesuń wskaźnik
                    printf("Dostawca Z: dostarczono jeden podzespół Z.\n");
                    break;
                }
                shm->z_delivery_addr += UNIT_SIZE_Z;
            }

            shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];  // Resetuj wskaźnik dla Z

        }

        // Odblokowanie dostępu do magazynu
        sem_op(semid, SEM_MUTEX, 1);

        if (is_magazyn_full(shm)) {  // Poprawnie wywołanie funkcji is_magazyn_full
            if (semctl(semid, SEM_MONTER_DONE, GETVAL) == 0) {
                printf("\033[34mKończę pracę! Brak miejsca w magazynie!\033[0m\n");
                exit(0);
            }
        }
        
    }
    sem_op(semid, SEM_DELIVERY_DONE, -1);

}

