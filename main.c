#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "magazyn.h"
#include "dostawca.h"
#include "monter.h"

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

    // Inicjalizacja pamięci magazynu (ustawienie wszystkich komórek na pustą wartość)
    memset(shm->magazyn, '\0', MAX_SPACE);

    // Inicjalizacja wskaźników do magazynu
    shm->x_pickup_addr = &shm->magazyn[0];  // Pierwsza część magazynu
    shm->y_pickup_addr = &shm->magazyn[MAX_SPACE / 6];  // Druga część magazynu
    shm->z_pickup_addr = &shm->magazyn[MAX_SPACE / 2];  // Trzecia część magazynu

    // Inicjalizacja wskaźników do dostaw
    shm->x_delivery_addr = &shm->magazyn[0];
    shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];
    shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];

    // Tworzenie semafora
    int semid = semget(SHM_KEY, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semafora (1 - dostępny)
    semctl(semid, SEM_MUTEX, SETVAL, 1);

  pid_t pid_x, pid_y, pid_z, pid_a, pid_b;

    // Tworzenie procesów 

    if ((pid_y = fork()) == 0) {
        dostawca(semid, shm, 'Y');
        exit(0);
    }

    if ((pid_z = fork()) == 0) {
        dostawca(semid, shm, 'Z');
        exit(0);
    }

    if ((pid_a = fork()) == 0) {
        monter(semid, shm, 'A');  
        exit(0);
    }

    if ((pid_x = fork()) == 0) {
        dostawca(semid, shm, 'X');
        exit(0);
    }

    if ((pid_b = fork()) == 0) {
        monter(semid, shm, 'B');  
        exit(0);
    }

    // Czekaj na zakończenie procesów dziecka
    if (waitpid(pid_y, NULL, 0) == -1) {
        perror("waitpid pid_y");
    }
    if (waitpid(pid_z, NULL, 0) == -1) {
        perror("waitpid pid_z");
    }
    if (waitpid(pid_a, NULL, 0) == -1) {
        perror("waitpid pid_a");
    }
    if (waitpid(pid_x, NULL, 0) == -1) {
        perror("waitpid pid_x");
    }
    if (waitpid(pid_b, NULL, 0) == -1) {
        perror("waitpid pid_b");
    }

    // Sprzątanie zasobów
    cleanup(semid, shmid, shm);

    return 0;
}
