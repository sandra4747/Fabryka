#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "magazyn.h"
#include "dostawca.h"
#include "monter.h"
#include "dyrektor.h"

int main() {

    // Tworzenie segmentu pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666 | IPC_CREAT);
    check_error(shmid == -1, "Błąd przy tworzeniu segmentu pamięci");

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    check_error(shm == (void *)-1, "Błąd przy dołączaniu segmentu pamięci");

    // Inicjalizacja pamięci magazynu 
    FILE *file = fopen("magazyn.txt", "rb");  // Otwieramy plik w trybie binarnym
    check_error(file == NULL, "Błąd odczytu stanu magazynu");
    fread(shm->magazyn, 1, MAX_SPACE, file);  // Wczytujemy dane do magazynu
    fclose(file);


    // Inicjalizacja wskaźników do dostaw
    shm->x_delivery_addr = &shm->magazyn[0];
    shm->y_delivery_addr = &shm->magazyn[MAX_SPACE / 6];
    shm->z_delivery_addr = &shm->magazyn[MAX_SPACE / 2];

    // Tworzenie semaforow
    int semid = semget(SHM_KEY, 3, 0666 | IPC_CREAT);
    check_error(semid == -1, "Błąd przy semget");

    // Inicjalizacja semaforow 
    semctl(semid, SEM_MUTEX, SETVAL, 1);

    semctl(semid, SEM_MONTER_DONE, SETVAL, 2);

    semctl(semid, SEM_DELIVERY_DONE, SETVAL, 3);


    pid_t pid_x, pid_y, pid_z, pid_a, pid_b, pid_dyr;

if ((pid_y = fork()) == 0) {
    printf("Uruchomiono proces dostawcy Y\n");
    dostawca('Y');
    exit(0);
}
check_error(pid_y == -1, "Błąd przy fork() dla pid_y");

if ((pid_z = fork()) == 0) {
    printf("Uruchomiono proces dostawcy Z\n");
    dostawca('Z');
    exit(0);
}
check_error(pid_z == -1, "Błąd przy fork() dla pid_z");

if ((pid_a = fork()) == 0) {
    printf("Uruchomiono proces montera A\n");
    monter('A');  
    exit(0);
}
check_error(pid_a == -1, "Błąd przy fork() dla pid_a");

if ((pid_x = fork()) == 0) {
    printf("Uruchomiono proces dostawcy X\n");
    dostawca('X');
    exit(0);
}
check_error(pid_x == -1, "Błąd przy fork() dla pid_x");

if ((pid_b = fork()) == 0) {
    printf("Uruchomiono proces montera B\n");
    monter('B');  
    exit(0);
}
check_error(pid_b == -1, "Błąd przy fork() dla pid_b");

if ((pid_dyr = fork()) == 0) {
    printf("Uruchomiono proces dyrektora\n");
    dyrektor(pid_x, pid_y, pid_z, pid_a, pid_b);
    exit(0);
}
check_error(pid_dyr == -1, "Błąd przy fork() dla pid_dyr");


    // Czekaj na zakończenie procesów dziecka
    check_error(waitpid(pid_y, NULL, 0) == -1, "Błąd przy waitpid dla pid_y");
    check_error(waitpid(pid_z, NULL, 0) == -1, "Błąd przy waitpid dla pid_z");
    check_error(waitpid(pid_a, NULL, 0) == -1, "Błąd przy waitpid dla pid_a");
    check_error(waitpid(pid_x, NULL, 0) == -1, "Błąd przy waitpid dla pid_x");
    check_error(waitpid(pid_b, NULL, 0) == -1, "Błąd przy waitpid dla pid_b");
    check_error(waitpid(pid_dyr, NULL, 0) == -1, "Błąd przy waitpid dla pid_dyr");

    // Sprzątanie zasobów
    cleanup(semid,shmid,shm);

    return 0;
}
