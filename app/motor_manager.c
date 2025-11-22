#include "motor_manager.h"
#include "log.h"
#include <stdio.h>
void initialize_motor_hardware() {
    log_init("MOTOR", NULL);
    LOG_INFO("Initializing hardware interface...");
}
void start_motor() {
    LOG_INFO("-> Sending Start Signal");
}
void stop_motor() {
    LOG_INFO("-> Sending Stop Signal");
}