#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "magazyn.h"

int main() {
    // Tworzenie segmentu pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666 | IPC_CREAT);
    check_error(shmid == -1, "Błąd przy tworzeniu segmentu pamięci");

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    check_error(shm == (void *)-1, "Błąd przy dołączaniu segmentu pamięci");

    // Inicjalizacja pamięci magazynu 
    FILE *file = fopen("magazyn.txt", "rb"); // Otwieramy plik w trybie binarnym
    if (file == NULL) {
        // Plik nie istnieje, tworzymy nowy
        file = fopen("magazyn.txt", "wb");
        check_error(file == NULL, "Błąd tworzenia nowego pliku magazyn.txt");
        memset(shm->magazyn, 0, MAX_SPACE); // Inicjalizujemy pustą pamięć magazynu
        fwrite(shm->magazyn, 1, MAX_SPACE, file); // Zapisujemy pusty magazyn do pliku
    } else {
        fread(shm->magazyn, 1, MAX_SPACE, file); // Wczytujemy dane do magazynu
    }
    fclose(file);

    // Tworzenie semaforów
    int semid = semget(SHM_KEY, 3, 0666 | IPC_CREAT);
    check_error(semid == -1, "Błąd przy semget");

    // Inicjalizacja semaforów
    semctl(semid, SEM_MUTEX, SETVAL, 1);
    semctl(semid, SEM_MONTER_DONE, SETVAL, 0);
    semctl(semid, SEM_DELIVERY_DONE, SETVAL, 0);

    // Wywoływanie procesów
    pid_t pid_x = -1, pid_y = -1, pid_z = -1, pid_a = -1, pid_b = -1, pid_dyr = -1;
    char pid_x_str[10], pid_y_str[10], pid_z_str[10], pid_a_str[10], pid_b_str[10];

    if ((pid_x = fork()) == 0) {
        execl("./dostawca", "dostawca", "X", NULL);
        perror("Błąd przy execl dla dostawcy X");
        exit(1);
    }
    check_error(pid_x == -1, "Błąd przy fork() dla pid_x");
    if (pid_x != -1) sem_op(semid, SEM_DELIVERY_DONE, 1);

    if ((pid_y = fork()) == 0) {
        execl("./dostawca", "dostawca", "Y", NULL);
        perror("Błąd przy execl dla dostawcy Y");
        exit(1);
    }
    check_error(pid_y == -1, "Błąd przy fork() dla pid_y");
    if (pid_y != -1) sem_op(semid, SEM_DELIVERY_DONE, 1);

    if ((pid_z = fork()) == 0) {
        execl("./dostawca", "dostawca", "Z", NULL);
        perror("Błąd przy execl dla dostawcy Z");
        exit(1);
    }
    check_error(pid_z == -1, "Błąd przy fork() dla pid_z");
    if (pid_z != -1) sem_op(semid, SEM_DELIVERY_DONE, 1);

    if ((pid_a = fork()) == 0) {
        execl("./monter", "monter", "A", NULL);
        perror("Błąd przy execl dla montera A");
        exit(1);
    }
    check_error(pid_a == -1, "Błąd przy fork() dla pid_a");
    if (pid_a != -1) sem_op(semid, SEM_MONTER_DONE, 1);

    if ((pid_b = fork()) == 0) {
        execl("./monter", "monter", "B", NULL);
        perror("Błąd przy execl dla montera B");
        exit(1);
    }
    check_error(pid_b == -1, "Błąd przy fork() dla pid_b");
    if (pid_b != -1) sem_op(semid, SEM_MONTER_DONE, 1);

    // Dyrektor
    if ((pid_dyr = fork()) == 0) {
        sprintf(pid_x_str, "%d", (pid_x != -1 ? pid_x : 0)); // Ustaw 0, jeśli pid_x nie istnieje
        sprintf(pid_y_str, "%d", (pid_y != -1 ? pid_y : 0)); // Ustaw 0, jeśli pid_y nie istnieje
        sprintf(pid_z_str, "%d", (pid_z != -1 ? pid_z : 0)); // Ustaw 0, jeśli pid_z nie istnieje
        sprintf(pid_a_str, "%d", (pid_a != -1 ? pid_a : 0)); // Ustaw 0, jeśli pid_a nie istnieje
        sprintf(pid_b_str, "%d", (pid_b != -1 ? pid_b : 0)); // Ustaw 0, jeśli pid_b nie istnieje

        execl("./dyrektor", "dyrektor", pid_x_str, pid_y_str, pid_z_str, pid_a_str, pid_b_str, NULL);
        perror("Błąd przy execl dla dyrektora");
        exit(1);
    }
    check_error(pid_dyr == -1, "Błąd przy fork() dla pid_dyr");

    // Oczekiwanie na zakończenie procesów
    if (pid_x != -1) check_error(waitpid(pid_x, NULL, 0) == -1, "Błąd przy waitpid dla pid_x");
    if (pid_y != -1) check_error(waitpid(pid_y, NULL, 0) == -1, "Błąd przy waitpid dla pid_y");
    if (pid_z != -1) check_error(waitpid(pid_z, NULL, 0) == -1, "Błąd przy waitpid dla pid_z");
    if (pid_a != -1) check_error(waitpid(pid_a, NULL, 0) == -1, "Błąd przy waitpid dla pid_a");
    if (pid_b != -1) check_error(waitpid(pid_b, NULL, 0) == -1, "Błąd przy waitpid dla pid_b");
    if (pid_dyr != -1) check_error(waitpid(pid_dyr, NULL, 0) == -1, "Błąd przy waitpid dla pid_dyr");

    // Sprzątanie zasobów
    cleanup(semid, shmid, shm);

    return 0;
}
