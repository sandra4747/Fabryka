#ifndef MAGAZYN_H
#define MAGAZYN_H

#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>

#define SEM_MUTEX 0
#define SEM_MONTER_DONE 1
#define SEM_DELIVERY_DONE 2

#define SHM_KEY 1234567
#define SEM_KEY 5678768
#define RAW_MAX_SPACE 100  // Oryginalna wartość magazynu
#define MAX_SPACE ((RAW_MAX_SPACE / 6) * 6)  // Dopasowanie do podzielności przez 6

#define UNIT_SIZE_X 1
#define UNIT_SIZE_Y 2
#define UNIT_SIZE_Z 3

typedef struct {
    char magazyn[MAX_SPACE];      
    
} SharedMemory;

void sem_op(int semid, int semnum, int op);
void cleanup(int semid, int shmid, SharedMemory *shm);
int is_any_section_empty(SharedMemory *shm);
int is_magazyn_full(SharedMemory *shm);
void check_error(int condition, const char *message);

#endif