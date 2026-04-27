#define _POSIX_C_SOURCE 200809L

#include "structures.h"

/* GLOBAL VARIABLES */
int returnValue_request; // Used to store the return value of the request fifo creation
int tub_nomme_request; // Used to store the request fifo file descriptor
int tub_nomme_answer; // Used to store the answer fifo file descriptor
int shm_fd; // Used for the shared memory file descriptor 
char PIPE_ANSWER[256]; // used to store the answer pipe fifo file name
sem_t *semlock;

/* Prototypes */
status_t parking_state_add(parking_state_t *, request_t );
void remove_at(parking_state_t *, int);
void sig_handler(int);
const char *action_to_string(action_t action);
const char *status_to_string(status_t status);


int main(int argc, char * argv[]){
    request_t client_req;
    response_t client_res;
    parking_state_t *pParking_state;
    char log_buf[512];

    /* Activate custom signal handler for SIGNINT */
    signal(SIGINT, sig_handler);

    /* Check if the argument count matches the wanted one */
    if (argc < 2){
        printf("Error : Insufficient arguments, need parking capacity\n");
        log_message("[SERVER][ERROR] Insufficient arguments, parking capacity missing");
        exit(EXIT_FAILURE);
    } else if (argc > 2){
        printf("Error : Too many arguments, enter parking capacity only\n");
        log_message("[SERVER][ERROR] Too many arguments provided at startup");
        exit(EXIT_FAILURE);
    }

    /* Opening SHM for parking state */
    shm_unlink(SHM_PARKING_STATE); // Unlink to delete and create a new shm file if one existed from an unexpected crash
    log_message("[SERVER][INFO] Unlinked previous SHM if it existed");

    // Create and open the shared memory
    shm_fd = shm_open(SHM_PARKING_STATE, O_CREAT | O_RDWR, 0600);
    if (shm_fd == -1){
        perror(SHM_PARKING_STATE);
        log_message("[SERVER][ERROR] Failed to create shared memory");
        exit(EXIT_FAILURE);
    } else {
        printf("SHM created successfully\n");
        log_message("[SERVER][INFO] Shared memory created successfully");
    }
    
    // Dimensioning
    if ((ftruncate(shm_fd, sizeof(parking_state_t))) == -1){
        printf("Truncate error : ");
        perror(SHM_PARKING_STATE);
        log_message("[SERVER][ERROR] Failed to size shared memory (ftruncate)");
        exit(EXIT_FAILURE);
    }

    // Mapping local variable to SHM
    pParking_state = mmap(NULL, sizeof(parking_state_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (pParking_state == MAP_FAILED){
        printf("Mapping error : ");
        perror(SHM_PARKING_STATE);
        log_message("[SERVER][ERROR] Failed to map shared memory");
        exit(EXIT_FAILURE);
    }
    log_message("[SERVER][INFO] Shared memory mapped successfully");

    /* Request fifo file creation */
    returnValue_request = mkfifo(PIPE_REQUESTS, 0600);
    if (returnValue_request == -1){
        printf("Request pipe file already exists, working with the already existing file...\n");
        log_message("[SERVER][WARNING] Request FIFO already existed, reusing it");
    } else {
        printf("Request pipe file created successfully !\n");
        log_message("[SERVER][INFO] Request FIFO created successfully");
    }

    /* Request fifo file opening for reading */
    tub_nomme_request = open(PIPE_REQUESTS, O_RDONLY);
    if (tub_nomme_request == -1){
        printf("Error : Unable to open request pipe file, server shutting down...\n");
        log_message("[SERVER][ERROR] Unable to open request FIFO, shutting down");
        close(tub_nomme_request);
        unlink(PIPE_REQUESTS);
        shm_unlink(SHM_PARKING_STATE);
        exit(EXIT_FAILURE);
    }
    log_message("[SERVER][INFO] Request FIFO opened for reading");

    /* Initialising the parking state */
    pParking_state->capacity = atoi(argv[1]);
    pParking_state->num_cars = 0;
    snprintf(log_buf, sizeof(log_buf),
             "[SERVER][INFO] Parking initialised with capacity %d", pParking_state->capacity);
    log_message(log_buf);


    /* Creating and opening the semaphore */
    semlock = sem_open(SEM_SHM_PARKING_STATE, O_CREAT, 0644, 1);
    if (semlock == SEM_FAILED){ // Can't open semaphore, clean up and exit
        printf("Error : can't open semaphore : \n");
        perror("sem_open");
        printf("\nServer shutting down...\n");
        log_message("[SERVER][ERROR] Failed to open semaphore, shutting down");
        
        // Clean up
        close(tub_nomme_request);
        unlink(PIPE_REQUESTS);
        shm_unlink(SHM_PARKING_STATE);

        exit(EXIT_FAILURE);
    }
    log_message("[SERVER][INFO] Semaphore created and opened successfully");

    /* Print server started successfully and waiting on the first request */
    printf("Sever started successfully and waiting on the first request !\n");
    log_message("[SERVER][INFO] Server started successfully, waiting for first request");


    while(1){
        
        /* Read the clients request and store it in client_req */
        ssize_t bytes_read = read(tub_nomme_request, &client_req, sizeof(request_t));
        
        // Check if there are any requests, meaning bytes read are more than 0
        if (bytes_read <= 0){ // Nothing to read
            continue; // Skip and go back to waiting
        }

        /* Print and log the request in human readable format in the server process */
        printf("Request received from client :\n ");
        print_request(client_req);
        printf("Working on the request...\n");

        snprintf(log_buf, sizeof(log_buf),
                 "[SERVER][REQUEST] Received from PID %d : car_id=%d action=%s",
                 client_req.client_pid,
                 client_req.car_id,
                 action_to_string(client_req.action));
        log_message(log_buf);

        /* Get the answer fifo file name and change the file descriptor */
        sprintf(PIPE_ANSWER, "/tmp/parking_answer_%d", client_req.client_pid);
        snprintf(log_buf, sizeof(log_buf),
                 "[SERVER][INFO] Answer FIFO resolved to %s", PIPE_ANSWER);
        log_message(log_buf);

        /* Answer fifo file opening for writing */
        tub_nomme_answer = open(PIPE_ANSWER, O_WRONLY);
        if (tub_nomme_answer == -1){
            printf("Error : Unable to open answer pipe file, server shutting down...\n");

            snprintf(log_buf, sizeof(log_buf),
                     "[SERVER][ERROR] Unable to open answer FIFO %s, shutting down", PIPE_ANSWER);
            log_message(log_buf);

            /* Clean up*/
            close(tub_nomme_answer);
            close(tub_nomme_request);
            unlink(PIPE_REQUESTS);
            shm_unlink(SHM_PARKING_STATE);
            sem_close(semlock);
            sem_unlink(SEM_SHM_PARKING_STATE);
        
            exit(EXIT_FAILURE);
        }

        /* Ask for the token before calling parking_state_add */
        sem_wait(semlock);
        log_message("[SERVER][INFO] Semaphore acquired");

        client_res.car_id = client_req.car_id;
        client_res.status = parking_state_add(pParking_state, client_req);

        /* Release token when done with modifying Parking_state struct */
        sem_post(semlock);
        log_message("[SERVER][INFO] Semaphore released");

        /* Log response */
        snprintf(log_buf, sizeof(log_buf),
                 "[SERVER][RESPONSE] car_id=%d action=%s status=%s occupancy=%d/%d",
                 client_res.car_id,
                 action_to_string(client_req.action),
                 status_to_string(client_res.status),
                 pParking_state->num_cars,
                 pParking_state->capacity);
        log_message(log_buf);

        /* Print the response in human readable format */
        printf("Done working on the request, sending response :\n");
        print_response(client_res);

        /* Send the response from the server to the client through the answer pipe */
        write(tub_nomme_answer, &client_res, sizeof(response_t));

        /* Clean up */
        close(tub_nomme_answer);
        log_message("[SERVER][INFO] Answer FIFO closed, waiting for next request");

        /* Print waiting on another request */
        printf("Waiting on another request...\n");
    }

    /* Clean up */
    close(tub_nomme_request);
    unlink(PIPE_REQUESTS);
    shm_unlink(SHM_PARKING_STATE);
    sem_close(semlock);
    sem_unlink(SEM_SHM_PARKING_STATE);
    
    exit(EXIT_SUCCESS);
}

/* Helper functions */

status_t parking_state_add(parking_state_t *pParking_state, request_t client_req){
    // add a car to the struct and check if it's there or not and send back the status of the car
    if (client_req.action == ENTER){ // Requesting entry to the parking
        
        // Check if the car is already parked
        for (int i = 0; i < pParking_state->num_cars; i++){
            // Go through the array and check if the car is already parked
            if (pParking_state->car_ids[i] == client_req.car_id){
                return ALREADY_PARKED;
            }
        }
        
        // Check if the parking is full
        if (pParking_state->num_cars == pParking_state->capacity){
            return FULL;
        }
        

        // Parking not full, car not parked => add the car to the parking and send back SUCCESS
        pParking_state->car_ids[pParking_state->num_cars] = client_req.car_id; // Add the car ID to the array of cars present in the parking
        pParking_state->num_cars++; // Increment the number of cars in the parking by 1
        return SUCCESS;

    } else if ( client_req.action == EXIT){ // Requesting exit from the parking
        
        // Check if there are no cars parked
        if (pParking_state->num_cars == 0){
            return NOT_FOUND;
        }

        // Look for the car
        for (int i = 0; i < pParking_state->num_cars; i++){
            if (pParking_state->car_ids[i] == client_req.car_id){ // Car found, remove it from the array (and shift all elements) and send back SUCCESS
                remove_at(pParking_state, i);
                return SUCCESS;
            }
        }

        // Car not found, send back NOT_FOUND
        return NOT_FOUND;
    }

    return SUCCESS; // To pass warning
}

void remove_at(parking_state_t *pParking_state, int index) { // Remove element at index; from car_ids array and shift elements beyond it left 
    if (index < 0 || index >= pParking_state->num_cars) return;

    // Shift elements to the left
    for (int i = index; i < pParking_state->num_cars - 1; i++) {
        pParking_state->car_ids[i] = pParking_state->car_ids[i + 1];
    }

    pParking_state->num_cars--; // Decrement size
}

void sig_handler(int sig){
    if (sig == SIGINT){ // If the signal received is SIGINT
        printf("\nSIGINT received, shutting down server...\n");
        if (tub_nomme_answer > 0){ // Check if there is an answer pipe open, if yes then the file descriptor is 0 and we need to close it
            close(tub_nomme_answer);
            unlink(PIPE_ANSWER);
        }
        close(tub_nomme_request);
        unlink(PIPE_REQUESTS);
        shm_unlink(SHM_PARKING_STATE);
        sem_close(semlock);
        sem_unlink(SEM_SHM_PARKING_STATE);
        exit(EXIT_SUCCESS);
    }
}

/* Function to transform action_t to string */
const char *action_to_string(action_t action){
    switch (action){
        case ENTER:   return "ENTER";
        case EXIT:    return "EXIT";
        default:      return "UNKNOWN";
    }
}
 
/* Function to transform status_t to string */
const char *status_to_string(status_t status){
    switch (status){
        case SUCCESS:        return "SUCCESS";
        case FULL:           return "FULL";
        case NOT_FOUND:      return "NOT_FOUND";
        case ALREADY_PARKED: return "ALREADY_PARKED";
        default:             return "UNKNOWN";
    }
}
