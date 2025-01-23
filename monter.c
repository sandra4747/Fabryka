#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "monter.h"
#include "magazyn.h"


volatile int flag_m = 1;

void handle_sigusr2(int sig) {
    flag_m = 0;
}

int main(int argc, char *argv[]) {
    check_error(argc != 2, "Błąd: niepoprawna liczba argumentów. Użycie: monter <A|B>");

    char stanowisko = argv[1][0];
    check_error(stanowisko != 'A' && stanowisko != 'B', "Błąd: niepoprawny typ montera. Użycie: monter <A|B>");

    // Łączenie się z istniejącym segmentem pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    check_error(shmid == -1, "Błąd przy przyłączaniu segmentu pamięci");

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    check_error(shm == (void *)-1, "Błąd przy dołączaniu segmentu pamięci");

    // Łączenie się z istniejącymi semaforami 
    int semid = semget(SHM_KEY, 3, 0666);  
    check_error(semid == -1, "Błąd przy semget");

    check_error(signal(SIGUSR2, handle_sigusr2) == SIG_ERR, "Błąd przy ustawianiu handlera sygnału SIGUSR2");

    srand(time(NULL) ^ getpid());

    // Wskaźniki lokalne dla każdego montera
    char *monter_x_pickup_addr = &shm->magazyn[0];
    char *monter_y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  
    char *monter_z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];


    while (flag_m) {
       usleep(rand() % 500000 + 300000);  // Opóźnienie montażu w zakresie od 0.3 do 0.8 sekundy

        sem_op(semid, SEM_MUTEX, -1);  // Zablokowanie dostępu do magazynu

        char found_x = 0, found_y = 0, found_z = 0;
        char *temp_x = NULL, *temp_y = NULL, *temp_z = NULL;

        // Szukanie komponentu X
        while (monter_x_pickup_addr < &shm->magazyn[MAX_SPACE / 6]) {
            if (*monter_x_pickup_addr == 'X') {
                *monter_x_pickup_addr = '\0';  // Odbiór podzespołu X
                temp_x = monter_x_pickup_addr;  // Zapisujemy adres komponentu X
                found_x = 1;
                break;
            }
            monter_x_pickup_addr += UNIT_SIZE_X;  // Zawsze przesuwamy wskaźnik
        }

        // Warunki dla komponentu X
        if (!found_x || monter_x_pickup_addr >= &shm->magazyn[MAX_SPACE / 6]) {
            monter_x_pickup_addr = &shm->magazyn[0];  // Reset wskaźnika w sekcji X
        }

        // Szukanie komponentu Y
        while (monter_y_pickup_addr < &shm->magazyn[MAX_SPACE / 2]) {
            if (*monter_y_pickup_addr == 'Y') {
                *monter_y_pickup_addr = '\0';  // Odbiór podzespołu Y
                temp_y = monter_y_pickup_addr;
                found_y = 1;
                break;
            }
            monter_y_pickup_addr += UNIT_SIZE_Y;  // Zawsze przesuwamy wskaźnik
        }

        // Warunki dla komponentu Y
        if (!found_y || monter_y_pickup_addr >= &shm->magazyn[MAX_SPACE / 2]) {
            monter_y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Reset wskaźnika w sekcji Y
        }

        // Szukanie komponentu Z
        while (monter_z_pickup_addr < &shm->magazyn[MAX_SPACE]) {
            if (*monter_z_pickup_addr == 'Z') {
                *monter_z_pickup_addr = '\0';  // Odbiór podzespołu Z
                temp_z = monter_z_pickup_addr;
                found_z = 1;
                break;
            }
            monter_z_pickup_addr += UNIT_SIZE_Z;  // Zawsze przesuwamy wskaźnik
        }

        // Warunki dla komponentu Z
        if (!found_z || monter_z_pickup_addr >= &shm->magazyn[MAX_SPACE]) {
            monter_z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];;  // Reset wskaźnika w sekcji Z
        }

        // Jeśli znaleziono wszystkie komponenty, zmontowano produkt
        if (found_x && found_y && found_z) {
            printf("Monter %c: Zmontowano jeden produkt.\n", stanowisko);
        } else {
            // Przywracamy komponenty, które nie zostały wzięte
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
                printf("\033[32mMonter %c: Kończę pracę! Brak komponentów w magazynie!\033[0m\n", stanowisko);
                check_error(shmdt(shm) == -1, "Błąd przy shmdt");
                exit(0);
            }
        }
    }

    // Monter kończy pracę, więc dekrementuje semafor
    sem_op(semid, SEM_MONTER_DONE, -1);
    check_error(shmdt(shm) == -1, "Błąd przy shmdt");  // Zakończenie pracy z pamięcią współdzieloną

}