#ifndef MAGAZYN_H
#define MAGAZYN_H

#include <sys/shm.h>
#include <sys/sem.h>

#define SEM_MUTEX 0

#define SHM_KEY 1234567
#define SEM_KEY 5678768
#define MAX_SPACE 90

#define UNIT_SIZE_X 1
#define UNIT_SIZE_Y 2
#define UNIT_SIZE_Z 3

typedef struct {
    char magazyn[MAX_SPACE];  
    
    char *x_delivery_addr;    
    char *y_delivery_addr;    
    char *z_delivery_addr;    

    char *x_pickup_addr;      
    char *y_pickup_addr;      
    char *z_pickup_addr;      
} SharedMemory;

void sem_op(int semid, int semnum, int op);
void cleanup(int semid, int shmid, SharedMemory *shm);

#endif
