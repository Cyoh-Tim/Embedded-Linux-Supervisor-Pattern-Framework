#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H
#include "common_ipc.h" // SystemState enum 사용

void initialize_state();
void set_system_state(SystemState state);
SystemState get_system_state();
#endif // STATE_MANAGER_H