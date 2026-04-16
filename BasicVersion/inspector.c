#include "structures.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Global variables */
sem_t *semlock;

/**
 * @brief Displays the current state of the parking lot.
 *
 * Prints the inspection time, capacity, number of parked cars, and their IDs.
 * Adds a delay for each car to simulate a slow inspector.
 *
 * @param parking_lot Pointer to the parking_state_t structure mapped from shared memory.
 */
void display_parking_state(parking_state_t *parking_lot) {
    // Get current date and time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("\n--- Parking Status ---\n");
    printf("Inspection Time: %s\n", time_str); // Print date and time
    printf("Capacity: %d\n", parking_lot->capacity);
    printf("Cars Parked: %d\n", parking_lot->num_cars);
        printf("----------"); fflush(stdout);
        sleep(1); // delay here to artificially slow the inspector
    printf("---------------\n");

    if (parking_lot->num_cars > 0) {
        printf("Parked Car IDs: "); fflush(stdout);
        for (int i = 0; i < parking_lot->num_cars; i++) {
            sleep(2); // delay here to artificially slow the inspector
            printf("%d ", parking_lot->car_ids[i]);
        }
        printf("\n");
    } else {
        printf("The parking lot is empty.\n");
    }
    sleep(1); // delay here to artificially slow the inspector
    printf("----------"); fflush(stdout);
    sleep(1); // delay here to artificially slow the inspector
    printf("---------------\n\n");
}

/**
 * @brief Entry point for the parking lot inspector utility.
 *
 * Connects to the shared memory segment created by the server,
 * maps it as read-only, and displays the current parking lot state.
 * Cleans up resources before exiting.
 *
 * @return EXIT_SUCCESS on success, or exits with an error code on failure.
 */
int main() {

    // Open the shared memory
    int shm_fd = shm_open(SHM_PARKING_STATE, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory
    parking_state_t *parking_lot = mmap(NULL, sizeof(parking_state_t), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (parking_lot == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    /* Opening the semaphore */
    semlock = sem_open(SEM_SHM_PARKING_STATE, 00);
    if (semlock == SEM_FAILED){
        printf("Error : can't open semaphore : \n");
        perror("sem_open");
        printf("\nInspector shutting down...\n");

        // Clean up
        close(shm_fd);

        exit(EXIT_FAILURE);
    }

    /* Ask for token before calling display_parking_state */
    sem_wait(semlock);

    // IfDisplay might show inconsistent state if server is running
    display_parking_state(parking_lot);

    /* Release token when done with accessing the shm through display_parking_state */
    sem_post(semlock);

    // Cleanup
    close(shm_fd);

    return EXIT_SUCCESS;
}
