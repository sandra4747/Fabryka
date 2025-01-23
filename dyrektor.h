#ifndef DYREKTOR_H
#define DYREKTOR_H

#include "magazyn.h"
#include <signal.h>

int is_process_alive(pid_t pid);
void send_signal_to_all_processes(pid_t pids[], size_t num_pids, int signal);
int save_magazyn_state(SharedMemory *shm);
char get_keypress(void);

#endif