#include "power_manager.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

//#include <fcntl.h>
//#include <unistd.h>
//#include <string.h>

// 실제 시스템에서는 여기에 GPIO 제어 라이브러리 등이 포함됩니다.

void initialize_power_manager_hardware() {
    log_init("POWER", NULL);
    LOG_INFO("Power Management IC initialized and ready.");
}

void trigger_hardware_power_off_signal() {
    LOG_FATAL(">>> FATAL: Sending PHYSICAL SHUTDOWN SIGNAL TO PMIC! <<<");
    // 실제 제품에서는 이 함수 호출 직후 시스템 전원이 차단됩니다.
    // 예시에서는 프로그램이 즉시 종료되도록 exit()를 호출합니다.
    exit(0); 
}

// I2C 사용 가정하에 보통 이런 식으로 저전력 MCU에 전원 차단을 지시합니다.
/*
void trigger_hardware_power_off_signal() {
    int fd;
    LOG_FATAL(">>> Sending PHYSICAL SHUTDOWN SIGNAL via GPIO! <<<");

    // 예시: GPIO 42번 핀을 사용한다고 가정
    // 1. GPIO 핀의 file descriptor를 엽니다. gpio2는 임의로 지정한 숫자입니다. 보통 몇번인지 SOC 사에서 알려줍니다.
    fd = open("/sys/class/gpio/gpio2/value", O_WRONLY);
    if (fd == -1) {
        LOG_ERROR("Failed to open GPIO value file");
        exit(1);
    }

    // 2. '1' (HIGH 신호)을 써서 MCU에게 전원 차단을 지시합니다.
    if (write(fd, "1", 1) == -1) {
        LOG_ERROR("Failed to write to GPIO value file: %s" strerror(errno));
        close(fd);
        exit(1);
    }
    
    close(fd);

    // 신호를 보낸 후, MCU가 전원을 차단할 시간을 줍니다.
    usleep(100000); // 100ms 대기

    // 이 시점에서 MCU에 의해 메인 전원이 끊어집니다.
    exit(0); 
}
*/