// Compilation command: gcc -o mydaemon mydaemon.c

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

#define SUCCESS             0
#define ERROR_SETSID        1
#define ERROR_FORK          2
#define ERROR_CHDIR         3
#define ERROR_UNKNOWN       9
#define DAEMON_NAME         "mydaemon"

// Function to handle signals received by the daemon
void handle_signal(int sig) {
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

// Function to log the current system time every second
void log_current_time(void) {
    while (1) {
        time_t now = time(NULL);
        syslog(LOG_INFO, "Current Time: %s", ctime(&now));
        sleep(1);
    }
}

// Function to initialize the daemon process
void initialize_daemon(void) {
    pid_t pid = fork();

    if (pid < 0) {
        syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
        exit(ERROR_FORK);
    }

    if (pid > 0) {
        // Parent process exits after fork
        exit(SUCCESS);
    }

    // Create a new session to detach from the controlling terminal
    if (setsid() < 0) {
        syslog(LOG_ERR, "setsid failed: %s", strerror(errno));
        exit(ERROR_SETSID);
    }

    // Change working directory to root to avoid directory lock issues
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "chdir to root failed: %s", strerror(errno));
        exit(ERROR_CHDIR);
    }

    // Close standard file descriptors to detach from terminal
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Set file permissions created by the daemon
    umask(0);
}

int main() {
    // Open system log for the daemon
    openlog(DAEMON_NAME, LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Daemon starting up.");

    // Initialize the daemon
    initialize_daemon();

    // Set up signal handling
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);

    // Start the main functionality of the daemon
    log_current_time();

    // Close system log before exiting
    closelog();
    return SUCCESS;
}
