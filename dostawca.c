#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>

#define SHM_KEY 1234
#define SEM_MUTEX 0
#define SEM_EMPTY 1
#define SEM_FULL 2

#define UNIT_SIZE_X 1
#define UNIT_SIZE_Y 2
#define UNIT_SIZE_Z 3

#define MAX_SPACE 90

typedef struct {
    char magazyn[MAX_SPACE];  
    char *x_delivery_addr;    // Wskaźnik na adres podzespołów X
    char *y_delivery_addr;    // Wskaźnik na adres podzespołów Y
    char *z_delivery_addr;    // Wskaźnik na adres podzespołów Z
} SharedMemory;

void sem_op(int semid, int semnum, int op) {
    struct sembuf operation = {semnum, op, 0};
    semop(semid, &operation, 1);
}

void cleanup(int semid, int shmid, SharedMemory *shm) {
    if (shmdt(shm) == -1) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

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

int main() {
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
    waitpid(pid_x, NULL, 0);
    waitpid(pid_y, NULL, 0);
    waitpid(pid_z, NULL, 0);

    cleanup(semid, shmid, shm);

    return 0;
}
