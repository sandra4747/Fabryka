#ifndef DYREKTOR_H
#define DYREKTOR_H

#include "magazyn.h"
#include <signal.h>

void dyrektor(int semid, pid_t pid_x, pid_t pid_y, pid_t pid_z, pid_t pid_a, pid_t pid_b, SharedMemory *shm);
int is_process_alive(pid_t pid);
void send_signal_to_all_processes(pid_t pids[], size_t num_pids, int signal);
int save_magazyn_state(SharedMemory *shm, int semid);
char get_keypress(void);

#endif 