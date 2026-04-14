#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

// --- System-wide Constants ---
#define SHM_PARKING_STATE "/parking_lot_shm"
#define PIPE_REQUESTS "/tmp/parking_requests"
#define MAX_CARS 10

// --- Semaphore Names ---
#define SEM_SHM_PARKING_STATE "/parking_sem"

// --- Data Structures ---
typedef enum {
    ENTER,
    EXIT
} action_t;

typedef enum {
    SUCCESS,
    FULL,
    NOT_FOUND,
    ALREADY_PARKED
} status_t;

/**
 * @brief Structure representing a client request.
 *
 * Contains the PID of the requesting client, the car ID, and the requested action (ENTER or EXIT).
 *
 * @example
 * // Example: create a fake request_t for testing
 * request_t fake_req;
 * fake_req.client_pid = 12345;      // Example PID
 * fake_req.car_id = 007;             // Example car ID
 * fake_req.action = ENTER;          // or EXIT
 *
 * // Or using designated initializer:
 * request_t fake_req2 = {
 *     .client_pid = 12345,
 *     .car_id = 007,
 *     .action = ENTER
 * };
 */
typedef struct {
    pid_t client_pid;    ///< Process ID of requesting client
    int car_id;          ///< ID of the car
    action_t action;     ///< Action to perform (ENTER or EXIT)
} request_t;

/**
 * @brief Structure representing a server response.
 *
 * Contains the car ID and the result status of the operation.
 *
 * @example
 * // Example: create a fake response_t for testing
 * response_t fake_resp;
 * fake_resp.car_id = 42;         // Example car ID
 * fake_resp.status = SUCCESS;    // Or FULL, NOT_FOUND, ALREADY_PARKED
 *
 * // Or using designated initializer:
 * response_t fake_resp2 = {
 *     .car_id = 007,
 *     .status = SUCCESS
 * };
 */
typedef struct {
    int car_id;        ///< ID of the car the response is for
    status_t status;   ///< Result of the operation
} response_t;

/**
 * @brief Structure representing the state of the parking lot.
 *
 * Contains the maximum capacity, the current number of cars, and the IDs of parked cars.
 *
 * @example
 * // Example: create a fake parking_state_t for testing
 * parking_state_t fake_state;
 * fake_state.capacity = 10;           // Example capacity
 * fake_state.num_cars = 2;            // Example: 2 cars parked
 * fake_state.car_ids[0] = 007;         // Example car IDs
 * fake_state.car_ids[1] = 42;
 *
 * // Or using designated initializer:
 * parking_state_t fake_state2 = {
 *     .capacity = 10,
 *     .num_cars = 2,
 *     .car_ids = {007, 42}
 * };
 */
typedef struct {
    int capacity;              ///< Maximum capacity of the parking lot
    int num_cars;              ///< Current number of cars in the lot
    int car_ids[MAX_CARS];     ///< Array to store the IDs of parked cars
} parking_state_t;



/**
 * @brief Prints the contents of a request_t structure in a human-readable format.
 *
 * @param req The request_t structure to print (contains client_pid, car_id, and action).
 */
void print_request(const request_t req);

/**
 * @brief Prints the contents of a response_t structure in a human-readable format.
 *
 * @param resp The response_t structure to print (contains car_id and status).
 */
void print_response(const response_t resp);

#endif // COMMON_H
