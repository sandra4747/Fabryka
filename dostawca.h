#ifndef DOSTAWCA_H
#define DOSTAWCA_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include "magazyn.h"  

void dostawca(int semid, SharedMemory *shm, char type);

#endif

