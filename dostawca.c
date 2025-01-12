#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "magazyn.h"
#include "dostawca.h"

void dostawca(int semid, SharedMemory *shm, char type) {
    while (1) {
        usleep(rand() % 100000 + 500000);  // Opóźnienie dostawy (zmniejszone)

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

            if (shm->x_delivery_addr >= &shm->magazyn[MAX_SPACE / 6]) {
                printf("Dostawca X: Brak miejsca w sekcji X.\n");
                shm->x_delivery_addr = &shm->magazyn[0];  // Resetuj wskaźnik, jeśli brak miejsca
                sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
                usleep(1000000);  // Dodatkowe czekanie, aby dać szansę monterowi na odbiór
                continue;  // Czekaj na zwolnienie miejsca
            }
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

            if (shm->y_delivery_addr >= &shm->magazyn[MAX_SPACE / 2]) {
                printf("Dostawca Y: Brak miejsca w sekcji Y.\n");
                shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];  // Resetuj wskaźnik dla Y
                sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
                usleep(1000000);  // Dodatkowe czekanie, aby dać szansę monterowi na odbiór
                continue;  // Czekaj na zwolnienie miejsca
            }
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

            if (shm->z_delivery_addr >= &shm->magazyn[MAX_SPACE]) {
                printf("Dostawca Z: Brak miejsca w sekcji Z. Czekam...\n");
                shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];  // Resetuj wskaźnik dla Z
                sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
                usleep(1000000);  // Dodatkowe czekanie, aby dać szansę monterowi na odbiór
                continue;  // Czekaj na zwolnienie miejsca
            }
        }

        // Odblokowanie dostępu do magazynu
        sem_op(semid, SEM_MUTEX, 1);
    }
}
