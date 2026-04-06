#ifndef LOGGER_H
#define LOGGER_H

/**
 * @brief Writes a message to the log file.
 * 
 * NOTE: This function is NOT process-safe by itself. The caller must
 * acquire and release the log semaphore (`SEM_LOG_KEY`) before and after
 * calling this function to prevent race conditions.
 * 
 * @param message The message string to write.
 */
void log_message(const char *message);

#endif // LOGGER_H
