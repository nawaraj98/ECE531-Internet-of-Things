// Compilation command: gcc -o mydaemon_hardened mydaemon_hardened.c
// To run: ./mydaemon_hardened



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>

#define SUCCESS             0
#define ERROR_SETSID        1
#define ERROR_FORK          2
#define ERROR_CHDIR         3
#define ERROR_UNKNOWN       9
#define DAEMON_NAME         "mydaemon"

void handle_signal(int sig, siginfo_t *siginfo, void *context) {
    switch (sig) {
        case SIGHUP:
            syslog(LOG_INFO, "Received SIGHUP signal.");
            break;
        case SIGTERM:
            syslog(LOG_INFO, "Received SIGTERM signal, shutting down.");
            closelog();
            exit(SUCCESS);
            break;
        default:
            syslog(LOG_INFO, "Received unhandled signal.");
            closelog();
            exit(ERROR_UNKNOWN);
    }
}

void log_current_time(void) {
    while (1) {
        time_t now = time(NULL);
        syslog(LOG_INFO, "Current Time: %s", ctime(&now));
        sleep(1);
    }
}

void initialize_daemon(void) {
    pid_t pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(ERROR_FORK);
    }

    if (pid > 0) {
        exit(SUCCESS);
    }

    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        exit(ERROR_SETSID);
    }

    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir to root failed: %s", strerror(errno));
        exit(ERROR_CHDIR);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    umask(0);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        if (rl.rlim_max == RLIM_INFINITY) {
            rl.rlim_max = 1024;
        }
        for (rlim_t i = 0; i < rl.rlim_max; i++) {
            close(i);
        }
    }
}

int main() {
    openlog(DAEMON_NAME, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon starting up.");

    initialize_daemon();

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = &handle_signal;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    log_current_time();

    closelog();
    return SUCCESS;
}
