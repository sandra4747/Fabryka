#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "magazyn.h"
#include "dostawca.h"

int main() {
    // Tworzenie segmentu pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Przypisanie wskaźników do odpowiednich miejsc w tablicy magazynu
    shm->x_delivery_addr = &shm->magazyn[0];
    shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];
    shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];

    // Tworzenie semaforów
    int semid = semget(SHM_KEY, 3, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semaforów
    semctl(semid, SEM_MUTEX, SETVAL, 1);
    semctl(semid, SEM_EMPTY, SETVAL, MAX_SPACE);  // Początkowo dostępne miejsce w magazynie
    semctl(semid, SEM_FULL, SETVAL, 0);  // Początkowo brak pełnych jednostek

    pid_t pid_x, pid_y, pid_z;

    // Tworzenie procesów dla dostawców X, Y, Z
    if ((pid_x = fork()) == 0) {
        dostawca(semid, shm, 'X');
        exit(0);
    }

    if ((pid_y = fork()) == 0) {
        dostawca(semid, shm, 'Y');
        exit(0);
    }

    if ((pid_z = fork()) == 0) {
        dostawca(semid, shm, 'Z');
        exit(0);
    }

    // Czekaj na zakończenie procesów dziecka
    if (waitpid(pid_x, NULL, 0) == -1) {
        perror("waitpid pid_x");
    }
    if (waitpid(pid_y, NULL, 0) == -1) {
        perror("waitpid pid_y");
    }
    if (waitpid(pid_z, NULL, 0) == -1) {
        perror("waitpid pid_z");
    }

    // Sprzątanie zasobów
    cleanup(semid, shmid, shm);

    return 0;
}
