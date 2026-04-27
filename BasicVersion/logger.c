/**
 * @file logger.c
 * @brief Logging utility for the parking lot system.
 *
 * Provides functions to write log messages to a file, typically used by the server and clients.
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
 #include <signal.h>


#define LOG_FILE_NAME "parking.log"

/**
 * @brief Writes a message to the log file with a timestamp.
 *
 * This function appends the given message to the log file, prefixing it with the current date and time.
 * NOTE: This function has a 1/6 probability of failing randomly to simulate logging errors.
 *
 * @param message The message to log.
 */
void log_message(const char *message) {
    // Initialize random seed if not already done
    static int seed_initialized = 0;
    if (!seed_initialized) {
        srand(time(NULL));
        seed_initialized = 1;
    }
    
    // Random failure: 1/6 probability
    int ale = (rand() % 3);
        fprintf(stderr, "Logger  %d\n", ale);
    if (ale == 0) {
        fprintf(stderr, "Logger failure !\n");
        raise((ale+1));
    }
    
    FILE *logfile = fopen(LOG_FILE_NAME, "a");
    if (!logfile) return;
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(logfile, "[%s] %s\n", time_str, message);
    fclose(logfile);
}