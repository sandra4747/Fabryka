#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>

#define SHM_KEY 1234     // Klucz pamięci dzielonej
#define SEM_MUTEX 0      // Indeks semafora mutex
#define SEM_EMPTY 1      // Indeks semafora "puste miejsce"
#define SEM_FULL 2       // Indeks semafora "pełne miejsce"

#define UNIT_SIZE_X 1    // Waga komponentu X w bajtach
#define UNIT_SIZE_Y 2    // Waga komponentu Y w bajtach
#define UNIT_SIZE_Z 3    // Waga komponentu Z w bajtach

#define MAX_SPACE 90     // Całkowita pojemność magazynu w bajtach

typedef struct {
    int x_count;           // Liczba komponentów X
    int y_count;           // Liczba komponentów Y
    int z_count;           // Liczba komponentów Z
    int total_space;       // Całkowita zajęta przestrzeń w bajtach
    int max_space;         // Maksymalna pojemność magazynu w bajtach (np. 90)
    int x_space;           // Zajęte miejsce przez X (maks. 15 bajtów)
    int y_space;           // Zajęte miejsce przez Y (maks. 30 bajtów)
    int z_space;           // Zajęte miejsce przez Z (maks. 45 bajtów)
} SharedMemory;

// Funkcja do zmiany wartości semafora
void sem_op(int semid, int semnum, int op) {
    struct sembuf operation = {semnum, op, 0};
    semop(semid, &operation, 1);
}

// Funkcja do czyszczenia zasobów
void cleanup(int semid, int shmid, SharedMemory *shm) {
    // Usunięcie pamięci dzielonej
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    // Usunięcie semaforów
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

// Funkcja dla dostawcy
void dostawca(int semid, SharedMemory *shm, char type) {
    int unit_size = (type == 'X') ? 1 : (type == 'Y') ? 2 : 3;  // Waga podzespołu w bajtach
    int max_unit_space = (type == 'X') ? 15 : (type == 'Y') ? 30 : 45;  // Maksymalna przestrzeń dla X, Y, Z

    while (1) {
        usleep(rand() % 1000000 + 500000);  // Losowa dostawa co 0.5–1 sekundy

        // Sprawdzanie, czy magazyn się nie przepełnił DO TESTOW
        if (shm->total_space >= shm->max_space) {
            printf("Magazyn pełny. Dostawca %c kończy pracę.\n", type);
            exit(0);  // Kończy działanie procesu dostawcy
        }

        sem_op(semid, SEM_EMPTY, -1);  // Czekaj na wolne miejsce
        sem_op(semid, SEM_MUTEX, -1);  // Wejdź do sekcji krytycznej

        // Sprawdzanie, czy jest wystarczająco miejsca w magazynie
        if (type == 'X' && shm->x_space + unit_size <= 15) {
            // Dodawanie komponentu X do magazynu
            shm->x_count++;
            shm->x_space += unit_size;
            shm->total_space += unit_size;
            printf("Dostawca X: dostarczono jeden podzespół.\n");
        } else if (type == 'Y' && shm->y_space + unit_size <= 30) {
            // Dodawanie komponentu Y do magazynu
            shm->y_count++;
            shm->y_space += unit_size;
            shm->total_space += unit_size;
            printf("Dostawca Y: dostarczono jeden podzespół.\n");
        } else if (type == 'Z' && shm->z_space + unit_size <= 45) {
            // Dodawanie komponentu Z do magazynu
            shm->z_count++;
            shm->z_space += unit_size;
            shm->total_space += unit_size;
            printf("Dostawca Z: dostarczono jeden podzespół.\n");
        } else {
            printf("Dostawca %c: Brak miejsca w magazynie.\n", type);
        }

        sem_op(semid, SEM_MUTEX, 1);  // Wyjdź z sekcji krytycznej
        sem_op(semid, SEM_FULL, 1);   // Dodaj pełne miejsce
    }
}


int main() {
    // Tworzenie pamięci dzielonej
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

    // Inicjalizacja pamięci dzielonej
    shm->x_count = 0;
    shm->y_count = 0;
    shm->z_count = 0;
    shm->total_space = 0;
    shm->max_space = MAX_SPACE;
    shm->x_space = 0;
    shm->y_space = 0;
    shm->z_space = 0;

    // Tworzenie semaforów
    int semid = semget(SHM_KEY, 3, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja semaforów
    semctl(semid, SEM_MUTEX, SETVAL, 1);   // Semafor mutex - początkowo dostępny
    semctl(semid, SEM_EMPTY, SETVAL, MAX_SPACE);  // Początkowo cała przestrzeń jest pusta
    semctl(semid, SEM_FULL, SETVAL, 0);     // Początkowo brak pełnych miejsc

    // Tworzenie trzech procesów dla różnych komponentów
    if (fork() == 0) {
        dostawca(semid, shm, 'X');  // Dostawca dla X
        exit(0);
    }

    if (fork() == 0) {
        dostawca(semid, shm, 'Y');  // Dostawca dla Y
        exit(0);
    }

    if (fork() == 0) {
        dostawca(semid, shm, 'Z');  // Dostawca dla Z
        exit(0);
    }

    // Czekanie na zakończenie wszystkich procesów
    wait(NULL);
    wait(NULL);
    wait(NULL);

    // Czyszczenie pamięci dzielonej i semaforów
    cleanup(semid, shmid, shm);

    return 0;
}
