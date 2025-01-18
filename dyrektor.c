#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "dyrektor.h"

// Funkcja do zapisu stanu magazynu
void save_magazyn_state(SharedMemory *shm) {
    FILE *file = fopen("magazyn.txt", "wb");
    if (file) {
        fwrite(shm->magazyn, 1, MAX_SPACE, file);
        fclose(file);
        printf("Stan magazynu zapisany do magazyn.txt.\n");
    } else {
        perror("Błąd zapisu stanu magazynu");
    }
}

// Funkcja wysyłająca sygnały do wszystkich procesów
void send_signal_to_all_processes(pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, int signal) {
    kill(pid_y, signal);
    kill(pid_z, signal);
    kill(pid_a, signal);
    kill(pid_x, signal);
    kill(pid_b, signal);
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

void dyrektor(pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, SharedMemory *shm) {
    char command;

    // Wyświetlenie menu
    printf("\nDyrektor: Wybierz polecenie:\n");
    printf("1 - Zatrzymanie magazynu\n");
    printf("2 - Zatrzymanie fabryki\n");
    printf("3 - Zatrzymanie fabryki i magazynu, zapis stanu magazynu\n");
    printf("4 - Zatrzymanie fabryki i magazynu bez zapisu stanu\n");

    while (1) {
        // Oczekiwanie na naciśnięcie klawisza
        command = get_keypress();

        if (command == '1') {
            printf("Dyrektor: Magazyn kończy pracę.\n");
            kill(pid_x, SIGUSR1);
            kill(pid_y, SIGUSR1);
            kill(pid_z, SIGUSR1);
        } else if (command == '2') {
            printf("Dyrektor: Fabryka kończy pracę.\n");
            kill(pid_a, SIGUSR2);
            kill(pid_b, SIGUSR2);
        } else if (command == '3') {
            printf("Dyrektor: Zapis stanu magazynu i zakończenie pracy.\n");

            // Zapis stanu magazynu
            save_magazyn_state(shm);

            // Zatrzymanie wszystkich procesów
            send_signal_to_all_processes(pid_x, pid_y, pid_z, pid_a, pid_b, SIGTERM);

            exit(0);
        } else if (command == '4') {
            printf("Dyrektor: Zakończenie pracy bez zapisu.\n");

            // Wyzerowanie stanu magazynu
            memset(shm->magazyn, '\0', MAX_SPACE);
            // Zapis wyzerowanego stanu magazynu do pliku
            save_magazyn_state(shm);

            printf("Dyrektor: Magazyn został opróżniony.\n");

            // Zatrzymanie wszystkich procesów
            send_signal_to_all_processes(pid_x, pid_y, pid_z, pid_a, pid_b, SIGTERM);

            exit(0);
        } else {
            printf("Dyrektor: Nieznane polecenie. Spróbuj ponownie.\n");
        }
    }
}
