#ifndef MONTER_H
#define MONTER_H

#include "magazyn.h"

void monter(int semid, SharedMemory *shm, char stanowisko);
void handle_sigusr2(int sig);

#endif
