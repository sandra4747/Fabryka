#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "monter.h"
#include "magazyn.h"

// void monter(int semid, SharedMemory *shm, char stanowisko) {
//     while (1) {
//         usleep(rand() % 1000000 + 900000);  // Opóźnienie dostawy

//         sem_op(semid, SEM_MUTEX, -1);  // Zablokuj dostęp do magazynu

//         // Zmienna do śledzenia, czy podzespoły zostały znalezione
//         int found_x = 0, found_y = 0, found_z = 0;

//         // Szukamy dostępnych komponentów
//         if (shm->x_pickup_addr < &shm->magazyn[MAX_SPACE / 6] && *shm->x_pickup_addr == 'X') {
//             *shm->x_pickup_addr = '\0';  // Odbiór podzespołu X
//             shm->x_pickup_addr += UNIT_SIZE_X;  // Zaktualizowanie wskaźnika
//             found_x = 1;
//             printf("Monter %c: Odbiór podzespołu X\n", stanowisko);
//         }

//         if (shm->y_pickup_addr < &shm->magazyn[MAX_SPACE / 2] && *shm->y_pickup_addr == 'Y') {
//             *shm->y_pickup_addr = '\0';  // Odbiór podzespołu Y
//             shm->y_pickup_addr += UNIT_SIZE_Y;  // Zaktualizowanie wskaźnika
//             found_y = 1;
//             printf("Monter %c: Odbiór podzespołu Y\n", stanowisko);
//         }

//         if (shm->z_pickup_addr < &shm->magazyn[MAX_SPACE] && *shm->z_pickup_addr == 'Z') {
//             *shm->z_pickup_addr = '\0';  // Odbiór podzespołu Z
//             shm->z_pickup_addr += UNIT_SIZE_Z;  // Zaktualizowanie wskaźnika
//             found_z = 1;
//             printf("Monter %c: Odbiór podzespołu Z\n", stanowisko);
//         }

//         // Jeśli wszystkie podzespoły zostały znalezione, montujemy produkt
//         if (found_x && found_y && found_z) {
//             printf("Monter %c: Zmontowano jeden produkt.\n", stanowisko);
//             sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu po zakończeniu montażu
//         }

//         // Jeśli brakuje jakiegoś komponentu, czekamy
//         else {
//             printf("Monter %c: Brak podzespołów w magazynie. Czekam...\n", stanowisko);
//             sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu 
//             usleep(1000000);  // Dodatkowe opóźnienie, aby dać czas na dostawę
//             continue;  // Czekanie na dostępność komponentów
//         }

//         // Jeśli brakuje komponentów, resetowanie wskaźników dla dostawcy
//         if (!found_x) {
//             shm->x_pickup_addr = &shm->magazyn[0];  // Resetuj wskaźnik do początku sekcji X
//         }

//         if (!found_y) {
//             shm->y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Resetuj wskaźnik do początku sekcji Y
//         }

//         if (!found_z) {
//             shm->z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];  // Resetuj wskaźnik do początku sekcji Z
//         }
//     }
// }

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "monter.h"
#include "magazyn.h"

void monter(int semid, SharedMemory *shm, char stanowisko) {
    while (1) {
        usleep(rand() % 1000000 + 500000);  // Opóźnienie dostawy

        sem_op(semid, SEM_MUTEX, -1);  // Zablokuj dostęp do magazynu

        // Inicjalizowanie zmiennych do sprawdzania, czy podzespoły zostały znalezione
        char found_x = 0, found_y = 0, found_z = 0;

        // Szukaj pierwszego dostępnego podzespołu w sekcji X
        while (shm->x_pickup_addr < &shm->magazyn[MAX_SPACE / 6]) {
            if (*shm->x_pickup_addr == 'X') {
                *shm->x_pickup_addr = '\0';  // Odbiór podzespołu X
                shm->x_pickup_addr += UNIT_SIZE_X;  // Przesunięcie wskaźnika o 1 bajt
                found_x = 1;
                //printf("Monter %c: Odbiór podzespołu X\n", stanowisko);
                break;
            } else {
                shm->x_pickup_addr += UNIT_SIZE_X;
            }
        }

        if (!found_x) {
            //printf("Monter %c: Brak podzespołów w sekcji X. Czekam...\n", stanowisko);
            sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
            shm->x_pickup_addr = &shm->magazyn[0];  // Resetowanie wskaźnika w sekcji X
            continue;  // Kontynuowanie pętli, aby czekać na dostawę
        }

        // Szukaj pierwszego dostępnego podzespołu w sekcji Y
        while (shm->y_pickup_addr < &shm->magazyn[MAX_SPACE / 3]) {
            if (*shm->y_pickup_addr == 'Y') {
                *shm->y_pickup_addr = '\0';  // Odbiór podzespołu Y
                shm->y_pickup_addr += UNIT_SIZE_Y;  // Przesunięcie wskaźnika o 2 bajty
                found_y = 1;
                //printf("Monter %c: Odbiór podzespołu Y\n", stanowisko);
                break;
            } else {
                shm->y_pickup_addr += UNIT_SIZE_Y;
            }
        }

        if (!found_y) {
            //printf("Monter %c: Brak podzespołów w sekcji Y. Czekam...\n", stanowisko);
            sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
            shm->y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Resetowanie wskaźnika w sekcji Y
            continue;  // Kontynuowanie pętli, aby czekać na dostawę
        }

        // Szukaj pierwszego dostępnego podzespołu w sekcji Z
        while (shm->z_pickup_addr < &shm->magazyn[MAX_SPACE]) {
            if (*shm->z_pickup_addr == 'Z') {
                *shm->z_pickup_addr = '\0';  // Odbiór podzespołu Z
                shm->z_pickup_addr += UNIT_SIZE_Z;  // Przesunięcie wskaźnika o 3 bajty
                found_z = 1;
                //printf("Monter %c: Odbiór podzespołu Z\n", stanowisko);
                break;
            } else {
                shm->z_pickup_addr += UNIT_SIZE_Z;
            }
        }

        if (!found_z) {
            //printf("Monter %c: Brak podzespołów w sekcji Z. Czekam...\n", stanowisko);
            sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie magazynu
            shm->z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];  // Resetowanie wskaźnika w sekcji Z
            continue;  // Kontynuowanie pętli, aby czekać na dostawę
        }

        // Jeśli wszystkie podzespoły zostały znalezione, zakończ montaż
        if (found_x && found_y && found_z) {
            printf("Monter %c: Zmontowano jeden produkt.\n", stanowisko);
        }
        else{
            printf("Monter %c: Brak odpowiednich komponentow.\n", stanowisko);
        }

        sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu
    }
}
