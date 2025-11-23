#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "common_ipc.h"
#include "log.h"

#define TIMEOUT_SECONDS 5   // watchdog 수행 시 타임아웃


// 실행 파일 이름들을 저장하는 배열 (common_ipc.h의 NUM_MANAGERS 크기)
const char* manager_exec_names[NUM_MANAGERS] = {
    LED_MANAGER_EXEC,
    MOTOR_MANAGER_EXEC,
    STATE_MANAGER_EXEC,
    POWER_MANAGER_EXEC 
};

// 프로세스 감시 및 재시작 위한 PID 저장 인덱스
pid_t child_pids[NUM_MANAGERS];

/**
 * @brief 인덱스를 사용하여 특정 매니저 프로세스를 생성하고 실행합니다.
 * @param manager_index 실행할 매니저의 배열 인덱스
 */
pid_t start_manager(int manager_index) {
    pid_t pid = fork();
    if (pid == 0) {
        // 자식 프로세스 영역
        const char* exec_name = manager_exec_names[manager_index];
        LOG_INFO("Starting manager: %s", exec_name);
        
        // execl 호출: 현재 프로세스를 새 실행 파일로 대체
        execl(exec_name, exec_name, (char *)NULL);
        
        // execl이 성공하면 이 아래 코드는 실행되지 않습니다. 실패했을 경우에만 실행
        LOG_FATAL("execl failed: %s", strerror(errno)); 
        exit(1);
    } else if (pid > 0)
    {
        //부모 프로세스 영역
        return pid;
    } else if (pid == -1) {
        // fork 실패
        LOG_ERROR("fork failed for manager %s: %s", manager_exec_names[manager_index], strerror(errno));
        return -1;
    }
    return -1;
}

/**
 * @brief OS 종료 시그널(Ctrl+C 등)을 처리하는 핸들러
 */
void shutdown_handler(int signum) {
    LOG_INFO("\nReceived OS signal %d. Terminating application.", signum);
    
    // 실제 제품에서는 여기서 STATE 매니저에게 CMD_SHUTDOWN 메시지를 보내야 함
    // (예제 단순화를 위해 exit(0) 호출)
    exit(0); 
}

/**
 * @brief 자식 프로세스 사망 시그널을 처리, 재시작
 */
void child_death_handler(int sig) {
    pid_t dead_pid, new_pid;
    int dead_index;
    // While 루프로 전체 프로세스를 도는 이유: 좀비 프로세스를 방지하기 위함.
    // WNOHANG -> 기다리는 PID가 종료되지 않은 상태일 경우 반환 값으로 0을 받음, 종료되어 종료 상태를 회수가 가능할 땐 pid를 받음
    LOG_ERROR("Received signal: %s (%d)", strsignal(sig), sig);
    while((dead_pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        LOG_WARN("경고! 프로세스(PID:%d)가 사망했습니다. 다시 살려냅니다...", dead_pid);
        dead_index = -1;
        for(int i = 0; i < NUM_MANAGERS; i++) {
            if(child_pids[i] == dead_pid) {
                dead_index = i;
                break;
            }
        }
        if(dead_index != -1) {
            LOG_INFO("Dead Manager Index: %d (%s) Restart...", dead_index, manager_exec_names[dead_index]);
            new_pid = start_manager(dead_index);
            child_pids[dead_index] = new_pid;
            LOG_INFO("%s New PID:%d", manager_exec_names[dead_index], new_pid);
        }
    }
}

int main() {
    // 0. 로그 초기화
    log_init("Main", NULL); // 파일에 로그를 저장하려면 NULL 대신 파일 이름 넣으면 됨
    LOG_INFO("System booting start");

    // OS 시그널(SIGINT: Ctrl+C, SIGTERM: 시스템 종료)이 오면 shutdown_handler 함수 실행
    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    signal(SIGCHLD, child_death_handler);
    signal(SIGALRM, child_death_handler);

    key_t key;
    int msgid;
    IpcMessage rcv_watchdog_msg;
    int rcv_result;

    // 1. ipc.key 파일을 기준으로 키 생성
    if ((key = ftok("./ipc.key", 'A')) == -1) {
        LOG_ERROR("ftok failed"); exit(1);
    }

    // 2. 메시지 큐 생성
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        LOG_ERROR("msgget failed: %s", strerror(errno));
    }
    LOG_INFO("Message Queue ID: %d created.", msgid);

    // 3. 모든 매니저 프로세스 시작 (for 루프 사용)
    for(int i = 0; i < NUM_MANAGERS; i++)
    {
        pid_t pid = start_manager(i);
        child_pids[i] = pid;
        LOG_INFO("Started manager '%s' with PID: %d. (Index: %d)", manager_exec_names[i], pid, i);
    }
    
    sleep(1); // 자식 프로세스들이 완전히 시작될 시간을 줌

    // 4. 부팅 완료 신호 전송 (STATE 매니저에게 총괄 시작을 알림)
    // common_ipc.c의 헬퍼 함수 사용
    send_ipc_message(msgid, TYPE_STATE_MANAGER, CMD_BOOT_SEQUENCE, "boot_done");
    
    LOG_INFO("Sent 'boot_done'. System operational.");
    LOG_INFO("System running. Press Ctrl+C to stop.");

    // 5. 전원 ON 동안 메인 프로세스는 계속 Watchdog 수행
    while(1) {
        //pause(); // OS 시그널이 올 때까지 CPU를 사용하지 않고 효율적으로 대기
        for(int i = 1; i < TYPE_MANAGER_MAX - 1; i++)   // main ipc는 감시할 필요 없음
        {
            pid_t target_pid = child_pids[i - 1];
            send_ipc_message(msgid, i + 1, CMD_REQUEST_PING, "Request Ping");   // Ping 요창
            alarm(TIMEOUT_SECONDS); // 5초까지 기다립니다
            rcv_result = msgrcv(msgid, &rcv_watchdog_msg, sizeof(IpcMessage) - sizeof(long), TYPE_MAIN_MANAGER, 0);
            if (rcv_result == -1) { // 수신 실패
                if (errno == EINTR) {
                    LOG_WARN("Timeout (SIGALRM) occurred for manager IPC check KILL target Process mtype: %d (PID: %d)",
                        i, target_pid);
                    // 프로세스를 kill 하면 다음 메시지에서 SIGCHLD 발생
                    kill(target_pid, SIGKILL);
                } else {
                    LOG_FATAL("msgrcv fatal error: %s", strerror(errno));
                    goto exit_watchdog_loop;    // 통신 자체에 이상이 있으므로 시스템 종료
                }
            } else {    // 수신 성공
                alarm(0);
                if (rcv_watchdog_msg.command == CMD_SEND_PONG && 
                    strcmp(rcv_watchdog_msg.payload, "Send Pong") == 0) 
                {
                    // 수신이 잘 되는지 확인하고 싶을 때 열면 됩니다.
                    //LOG_INFO("System OK, %s (PID: %d)", manager_exec_names[i - 1], target_pid);
                }
                else {
                    // 수신은 되었으나 이상한 커맨드를 받았을 때
                    LOG_WARN("Received invalid Watchdog response: mtype=%d, PID:%d, CMD=%d, Payload=%s", 
                        rcv_watchdog_msg.mtype, target_pid, rcv_watchdog_msg.command, rcv_watchdog_msg.payload);
                }
            }
        }
        sleep(1);   // 1초 단위로 감시
    }

exit_watchdog_loop:
    // Watchdog 수신 시 IPC 오류가 생길 때 시스템 종료함
    // 이 코드는 pause()에 의해 실행되지 않습니다.
    return 0; 
}