/**
 * @file structures.c
 * @brief Utility functions for printing request and response structures in the parking lot system.
 */

#include "structures.h"
#include <stdio.h>

/**
 * @brief Prints the contents of a request_t structure in a human-readable format.
 *
 * @param req The request_t structure to print (contains client_pid, car_id, and action).
 */
void print_request(const request_t req) {
    const char *action_str = (req.action == ENTER) ? "ENTER" : "EXIT";
    fprintf(stderr,"Request from PID %d: car_id=%d, action=%s",
             req.client_pid, req.car_id, action_str);
}

/**
 * @brief Prints the contents of a response_t structure in a human-readable format.
 *
 * @param resp The response_t structure to print (contains car_id and status).
 */
void print_response(const response_t resp) {
    const char *status_str = "UNKNOWN";
    switch (resp.status) {
        case SUCCESS: status_str = "SUCCESS"; break;
        case FULL: status_str = "FULL"; break;
        case NOT_FOUND: status_str = "NOT_FOUND"; break;
        case ALREADY_PARKED: status_str = "ALREADY_PARKED"; break;
    }
    fprintf(stderr, "Response: car_id=%d, status=%s\n", resp.car_id, status_str);
}