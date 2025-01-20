#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "dyrektor.h"

int save_magazyn_state(SharedMemory *shm, int semid) {

    sem_op(semid, SEM_MUTEX, -1); // Zablokowanie dostępu do magazynu
    
    FILE *file = fopen("magazyn.txt", "wb");
    if (!file) {
        perror("Błąd otwarcia pliku");
        sem_op(semid, SEM_MUTEX, 1); // Odblokowanie magazynu
        return -1;  
    }

    size_t written = fwrite(shm->magazyn, 1, MAX_SPACE, file);
    if (written != MAX_SPACE) {
        perror("Błąd zapisu stanu magazynu");
        fclose(file);
        sem_op(semid, SEM_MUTEX, 1); // Odblokowanie magazynu
        return -2;  
    }

    fclose(file);
    sem_op(semid, SEM_MUTEX, 1); // Odblokowanie magazynu
    return 0;  // Sukces
}


void send_signal_to_all_processes(pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, int signal) {
    kill(pid_x, signal);
    kill(pid_y, signal);
    kill(pid_z, signal);
    kill(pid_a, signal);
    kill(pid_b, signal);

    // Oczekiwanie na zakończenie procesów
    waitpid(pid_x, NULL, 0);
    waitpid(pid_y, NULL, 0);
    waitpid(pid_z, NULL, 0);
    waitpid(pid_a, NULL, 0);
    waitpid(pid_b, NULL, 0);
}


// Funkcja odbierająca wciśnięty klawisz
char get_keypress(void) {
    struct termios oldt, newt;
    char ch;

    // Pobranie aktualnych ustawień terminala
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Wyłączenie trybu kanonicznego i wyłączenie echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Odczytanie wciśniętego klawisza
    ch = getchar();

    // Przywrócenie ustawień terminala
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void dyrektor(int semid, pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, SharedMemory *shm) {
    char command;

    // Wyświetlenie menu
    printf("\nDyrektor: Wybierz polecenie:\n");
    printf("1 - Zatrzymanie magazynu\n");
    printf("2 - Zatrzymanie fabryki\n");
    printf("3 - Zatrzymanie fabryki i magazynu, zapis stanu magazynu\n");
    printf("4 - Zatrzymanie fabryki i magazynu bez zapisu stanu\n---------------------------------------------------\n\n");

    while (1) {
        
        // Oczekiwanie na wciśnięcie klawisza
        command = get_keypress();

        if (command == '1') {
            kill(pid_x, SIGUSR1);
            kill(pid_y, SIGUSR1);
            kill(pid_z, SIGUSR1);
            while (semctl(semid, SEM_DELIVERY_DONE, GETVAL) != 0) { usleep(100); }
            printf("\033[1;31mDyrektor: Magazyn kończy pracę.\n\033[0m");

        } else if (command == '2') {
            kill(pid_a, SIGUSR2);
            kill(pid_b, SIGUSR2);
            while (semctl(semid, SEM_MONTER_DONE, GETVAL) != 0) { usleep(100); }
            printf("\033[1;31mDyrektor: Fabryka kończy pracę.\n\033[0m");

        } else if (command == '3') {

            // Zatrzymanie wszystkich procesów
            send_signal_to_all_processes(pid_x, pid_y, pid_z, pid_a, pid_b, SIGTERM);

            // Zapis stanu magazynu
            save_magazyn_state(shm, semid);

            printf("\033[1;31mDyrektor: Zapis stanu magazynu i zakończenie pracy.\n\033[0m");

            exit(0);

        } else if (command == '4') {

            // Zatrzymanie wszystkich procesów
            send_signal_to_all_processes(pid_x, pid_y, pid_z, pid_a, pid_b, SIGTERM);

            // Czyszczenie magazynu
            memset(shm->magazyn, '\0', MAX_SPACE);

            save_magazyn_state(shm, semid);

            printf("\033[1;31mDyrektor: Zakończenie pracy bez zapisu.\n\033[0m");

            exit(0);
            
        } else {
            printf("\033[1;31mDyrektor: Nieznane polecenie. Spróbuj ponownie.\n\033[0m");
        }
    }
}
