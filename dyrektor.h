#ifndef DYREKTOR_H
#define DYREKTOR_H

#include "magazyn.h"
#include <signal.h>

void dyrektor(int semid, pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, SharedMemory *shm);
void send_signal_to_all_processes(pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, int signal);
int save_magazyn_state(SharedMemory *shm, int semid);
char get_keypress(void);

#endif 