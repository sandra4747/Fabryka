#include "magazyn.h"
#include "dostawca.h"

void dostawca(int semid, SharedMemory *shm, char type) {
    while (1) {
        usleep(rand() % 1000000 + 500000);  // Opóźnienie dostawy

        sem_op(semid, SEM_EMPTY, -1);  // Czekaj na miejsce w magazynie
        sem_op(semid, SEM_MUTEX, -1);  // Zablokuj dostęp do magazynu

        int full = 0;

        // Sprawdzanie miejsca na magazynie dla każdego typu podzespołu
        if (type == 'X' && shm->x_delivery_addr + UNIT_SIZE_X <= &shm->magazyn[MAX_SPACE/6]) {
            *shm->x_delivery_addr = 'X';  // Zapisanie podzespołu X
            shm->x_delivery_addr += UNIT_SIZE_X;  // Przesunięcie wskaźnika
            printf("Dostawca X: dostarczono jeden podzespół X.\n");
        } else if (type == 'Y' && shm->y_delivery_addr + UNIT_SIZE_Y <= &shm->magazyn[MAX_SPACE/2]) {
            *shm->y_delivery_addr = 'Y';  // Zapisanie podzespołu Y
            shm->y_delivery_addr += UNIT_SIZE_Y;  // Przesunięcie wskaźnika
            printf("Dostawca Y: dostarczono jeden podzespół Y.\n");
        } else if (type == 'Z' && shm->z_delivery_addr + UNIT_SIZE_Z <= &shm->magazyn[MAX_SPACE]) {
            *shm->z_delivery_addr = 'Z';  // Zapisanie podzespołu Z
            shm->z_delivery_addr += UNIT_SIZE_Z;  // Przesunięcie wskaźnika
            printf("Dostawca Z: dostarczono jeden podzespół Z.\n");
        } else {
            printf("Dostawca %c: Brak miejsca w magazynie.\n", type);
            full = 1;
        }

        sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie dostępu do magazynu
        sem_op(semid, SEM_FULL, 1);    // Zgłoszenie dostępności podzespołów w magazynie

        if (full) {
            printf("Dostawca %c: kończy pracę z powodu braku miejsca.\n", type);
            exit(0);  // Zakończenie procesu dostawcy
        }
    }
}

