#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include "dyrektor.h"
#include "magazyn.h"

// Globalna zmienna do przechowywania oryginalnych ustawień terminala
struct termios original_termios;

// Funkcja do przywracania oryginalnych ustawień terminala
void restore_terminal_settings(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

// Funkcja obsługująca sygnał SIGTSTP 
void handle_suspend(int sig) {
    restore_terminal_settings();
    signal(SIGTSTP, SIG_DFL); 
    raise(SIGTSTP); // Wykonaj domyślne działanie SIGTSTP
}

// Funkcja obsługująca sygnał SIGCONT 
void handle_continue(int sig) {
    // Przywróć tryb niekanoniczny po wznowieniu
    struct termios newt = original_termios;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

int save_magazyn_state(SharedMemory *shm) {

    FILE *file = fopen("magazyn.txt", "wb");
    check_error(file == NULL, "Błąd otwarcia pliku");

    size_t written = fwrite(shm->magazyn, 1, MAX_SPACE, file);
    check_error(written != MAX_SPACE, "Błąd zapisu stanu magazynu");
    
    fclose(file);
    
    return 0;  // Sukces
}

int is_process_alive(pid_t pid) {
    // Sprawdza, czy proces o PID istnieje
    if (kill(pid, 0) == 0) {
        return 1;  // Proces istnieje
    } else if (errno == ESRCH) {
        return 0;  // Proces nie istnieje
    } else {
        perror("Błąd sprawdzania procesu");
        return -1;  // Inny błąd
    }
}


void send_signal_to_all_processes(pid_t pids[], size_t num_pids, int signal) {
    for (size_t i = 0; i < num_pids; i++) {
        if (pids[i] == 0) {
            // Ignoruj PID równy 0
            continue;
        }
        if (is_process_alive(pids[i])) {
            check_error(kill(pids[i], signal) == -1, "Błąd przy wysyłaniu sygnału kill");
        }
    }
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

int main(int argc, char *argv[]) {    
    char command;

    // Pobrane oryginalne ustawienia terminala
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(restore_terminal_settings); // Przywrócenie ustawień po zakończeniu programu

    // Ustawienie obsługi sygnałów
    signal(SIGTSTP, handle_suspend);
    signal(SIGCONT, handle_continue);
    
    // Łączenie się z istniejącym segmentem pamięci współdzielonej
    int shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0600);
    check_error(shmid == -1, "Błąd przy przyłączaniu segmentu pamięci");

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    check_error(shm == (void *)-1, "Błąd przy dołączaniu segmentu pamięci");

    // Łączenie się z istniejącymi semaforami 
    int semid = semget(SHM_KEY, 3, 0600);  
    check_error(semid == -1, "Błąd przy semget");

    // Przypisanie PID-ów z argumentów
    if (argc < 6) {
        fprintf(stderr, "Błąd: Niewystarczająca liczba argumentów. Oczekiwano 5 PID-ów.\n");
        exit(1);
    }

    pid_t pids[5];
    for (int i = 0; i < 5; i++) {
        pids[i] = (pid_t)atoi(argv[i + 1]);
    }

    // Wyświetlenie menu
    printf("\nDyrektor: Wybierz polecenie:\n");
    printf("1 - Zatrzymanie magazynu\n");
    printf("2 - Zatrzymanie fabryki\n");
    printf("3 - Zatrzymanie fabryki i magazynu, zapis stanu magazynu\n");
    printf("4 - Zatrzymanie fabryki i magazynu bez zapisu stanu\n---------------------------------------------------\n\n");

    while (1) {

        command = get_keypress();

        if (command == '1') {
            send_signal_to_all_processes(pids, 3, SIGUSR1);  // Procesy x, y, z (magazyn)
            while (semctl(semid, SEM_DELIVERY_DONE, GETVAL) != 0) { usleep(100); }
            printf("\033[1;31mDyrektor: Magazyn kończy pracę.\n\033[0m");

        } else if (command == '2') {
            send_signal_to_all_processes(pids + 3, 2, SIGUSR2);  // Procesy a, b (fabryka)
            while (semctl(semid, SEM_MONTER_DONE, GETVAL) != 0) { usleep(100);  }
            printf("\033[1;31mDyrektor: Fabryka kończy pracę.\n\033[0m");

        } else if (command == '3') {
            send_signal_to_all_processes(pids, 5, SIGTERM);  // Wszystkie procesy
            // Zapis stanu magazynu
            save_magazyn_state(shm);

            printf("\033[1;31mDyrektor: Zapis stanu magazynu i zakończenie pracy.\n\033[0m");

            break;

        } else if (command == '4') {
            send_signal_to_all_processes(pids, 5, SIGTERM);  // Wszystkie procesy
            // Czyszczenie magazynu 
            memset(shm->magazyn, '\0', MAX_SPACE);
            save_magazyn_state(shm);

            printf("\033[1;31mDyrektor: Zakończenie pracy bez zapisu.\n\033[0m");

            break;

        } else {
            printf("\033[1;31mDyrektor: Nieznane polecenie. Spróbuj ponownie.\n\033[0m");
        }
    }
    check_error(shmdt(shm) == -1, "Błąd przy shmdt");  // Zakończenie pracy z pamięcią współdzieloną
    exit(0);
}