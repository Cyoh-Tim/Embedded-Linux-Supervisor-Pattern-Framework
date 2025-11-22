#include "led_manager.h"
#include "log.h"
#include <stdio.h>
void initialize_led_hardware() {
    log_init("LED", NULL);
    LOG_INFO("Initializing hardware interface...");
}
void set_led_state(int state) {
    if (state == 1) {
        LOG_INFO("-> GPIO High: LED ON");
    } else {
        LOG_INFO("-> GPIO Low: LED OFF");
    }
}