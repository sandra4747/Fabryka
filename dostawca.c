#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "magazyn.h"

// Wątek dostawcy X
void* dostawca_x(void* arg) {
    while (1) {
        sleep(rand() % 5 + 1);  // Symulacja czasu dostawy (losowy czas)
        
        // Próba dodania podzespołu X do magazynu
        if (dodaj_do_magazynu("X")) {
            printf("Dostawca X: Podzespół X dostarczony.\n");
        } else {
            printf("Dostawca X: Brak miejsca w magazynie.\n");
        }
    }
    return NULL;
}

// Wątek dostawcy Y
void* dostawca_y(void* arg) {
    while (1) {
        sleep(rand() % 5 + 1);  // Symulacja czasu dostawy (losowy czas)
        
        // Próba dodania podzespołu Y do magazynu
        if (dodaj_do_magazynu("Y")) {
            printf("Dostawca Y: Podzespół Y dostarczony.\n");
        } else {
            printf("Dostawca Y: Brak miejsca w magazynie.\n");
        }
    }
    return NULL;
}

// Wątek dostawcy Z
void* dostawca_z(void* arg) {
    while (1) {
        sleep(rand() % 5 + 1);  // Symulacja czasu dostawy (losowy czas)
        
        // Próba dodania podzespołu Z do magazynu
        if (dodaj_do_magazynu("Z")) {
            printf("Dostawca Z: Podzespół Z dostarczony.\n");
        } else {
            printf("Dostawca Z: Brak miejsca w magazynie.\n");
        }
    }
    return NULL;
}

int main() {
    pthread_t thread_x, thread_y, thread_z;

    // Inicjalizujemy magazyn i semafory
    inicjalizuj_magazyn();

    // Tworzenie wątków dostawców
    pthread_create(&thread_x, NULL, dostawca_x, NULL);
    pthread_create(&thread_y, NULL, dostawca_y, NULL);
    pthread_create(&thread_z, NULL, dostawca_z, NULL);

    // Czekanie na zakończenie wątków
    pthread_join(thread_x, NULL);
    pthread_join(thread_y, NULL);
    pthread_join(thread_z, NULL);

    
    return 0;
}
