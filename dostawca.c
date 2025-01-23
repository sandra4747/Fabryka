#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "magazyn.h"
#include "dostawca.h"

volatile int flag_d = 1; 

void handle_sigusr1(int sig) {
    flag_d = 0;
}

int main(int argc, char *argv[]) {
    check_error(argc != 2, "Błąd: niepoprawna liczba argumentów. Użycie: dostawca <X|Y|Z>");

    char type = argv[1][0];
    check_error(type != 'X' && type != 'Y' && type != 'Z', "Błąd: niepoprawny typ dostawcy. Użycie: dostawca <X|Y|Z>");

    // Łączenie się z istniejącym segmentem pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0600);
    check_error(shmid == -1, "Błąd przy przyłączaniu segmentu pamięci");

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    check_error(shm == (void *)-1, "Błąd przy dołączaniu segmentu pamięci");

    // Łączenie się z istniejącymi semaforami 
    int semid = semget(SHM_KEY, 3, 0600);  
    check_error(semid == -1, "Błąd przy semget");

    check_error(signal(SIGUSR1, handle_sigusr1) == SIG_ERR, "Błąd przy ustawianiu handlera sygnału SIGUSR2");
    
    srand(time(NULL) ^ getpid());

    // Inicjalizacja wskaźników do dostaw
    char *x_delivery_addr = &shm->magazyn[0];
    char *y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];
    char *z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];

    // Sprawdzenie czy jest dostawca dla kazdego komponentu - jesli nie - kończenie procesu
    if (semctl(semid, SEM_DELIVERY_DONE, GETVAL) != 3) {
        printf("\033[1;31mDyrektor: Wszyscy dostawcy muszą pracować. Kończenie procesu.\n\033[0m");
        sem_op(semid, SEM_DELIVERY_DONE, -1);
        check_error(shmdt(shm) == -1, "Błąd przy shmdt"); 
        exit(0);
    }

    while (flag_d) {
        usleep(rand() % 600000 + 300000); // Opóźnienie (0.3 - 0.9 s)
                
        sem_op(semid, SEM_MUTEX, -1);  // Próba zablokowania semafora

        if (type == 'X') {
            if (*x_delivery_addr == '\0') {
                *x_delivery_addr = 'X'; // Dodanie elementu X
                printf("Dostawca X: dostarczono jeden podzespół X.\n");
            }
            x_delivery_addr += UNIT_SIZE_X;
            if (x_delivery_addr >= &shm->magazyn[MAX_SPACE / 6]) {
                x_delivery_addr = &shm->magazyn[0]; // Reset wskaźnika
            }
        } else if (type == 'Y') {
            if (*y_delivery_addr == '\0') {
                *y_delivery_addr = 'Y'; // Dodanie elementu Y
                printf("Dostawca Y: dostarczono jeden podzespół Y.\n");
            }
            y_delivery_addr += UNIT_SIZE_Y;
            if (y_delivery_addr >= &shm->magazyn[MAX_SPACE / 2]) {
                y_delivery_addr = &shm->magazyn[MAX_SPACE / 6]; // Reset wskaźnika
            }
        } else if (type == 'Z') {
            if (*z_delivery_addr == '\0') {
                *z_delivery_addr = 'Z'; // Dodanie elementu Z
                printf("Dostawca Z: dostarczono jeden podzespół Z.\n");
            }
            z_delivery_addr += UNIT_SIZE_Z;
            if (z_delivery_addr >= &shm->magazyn[MAX_SPACE]) {
                z_delivery_addr = &shm->magazyn[MAX_SPACE / 2]; // Reset wskaźnika
            }
        }
        
        sem_op(semid, SEM_MUTEX, 1);  // Odblokowanie semafora

        // Sprawdzenie stanu magazynu, jeśli monterzy zakończyli pracę
        if (semctl(semid, SEM_MONTER_DONE, GETVAL) == 0) {  
            if (is_magazyn_full(shm)) {
                printf("\033[34mKończę pracę! Brak miejsca w magazynie!\033[0m\n");
                check_error(shmdt(shm) == -1, "Błąd przy shmdt");
                exit(0);
            }
        }
    }

    sem_op(semid, SEM_DELIVERY_DONE, -1); // Informacja o zakończeniu pracy dostawcy
    check_error(shmdt(shm) == -1, "Błąd przy shmdt");  // Zakończenie pracy z pamięcią współdzieloną
}
