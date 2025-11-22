#include "state_manager.h"
#include "log.h"
#include <stdio.h>

static SystemState current_state = STATE_BOOTING;
void initialize_state() {
    current_state = STATE_BOOTING;
    log_init("STATE", NULL);
    LOG_INFO("Initializing hardware interface...");
}
void set_system_state(SystemState state) { 
    current_state = state; 
    LOG_INFO("System State Changed to %d", current_state);
}
SystemState get_system_state() { 
    return current_state; 
}