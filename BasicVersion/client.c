
#include "structures.h"

/* GLOBAL VARIABLES */
int returnValue_answer; // Used to store the return value of the answer fifo creation
int tub_nomme_answer; // used to store the answer fifo file descriptor
int tub_nomme_request; // used to store the request fifo file descriptor 
char PIPE_ANSWER[256]; // used to store the answer pipe fifo file name

/* Prototypes */
void client_req_set(request_t *, pid_t, int, char[]);
const char *action_to_string(action_t action);


int main(int argc, char * argv[]){

    request_t client_req;
    response_t client_res;
    char log_buf[512];

    /* Check if the argument count matches the wanted one */
    if (argc < 3){
        printf("Error : Insufficient arguments, need car id and action\n");
        log_message("[CLIENT][ERROR] Insufficient arguments, need car id and action");
        exit(EXIT_FAILURE);
    } else if (argc > 3){
        printf("Error : Too many arguments, enter car id and action only\n");
        log_message("[CLIENT][ERROR] Too many arguments provided");
        exit(EXIT_FAILURE);
    }

    // Client request struct setting to the correct values
    client_req_set(&client_req, getpid(), atoi(argv[1]), argv[2]);

    // Log request
    snprintf(log_buf, sizeof(log_buf),
             "[CLIENT][REQUEST] Sending request: PID=%d car_id=%d action=%s",
             client_req.client_pid,
             client_req.car_id,
             action_to_string(client_req.action));
    log_message(log_buf);

    //add the PID of the client process to distinguish answer fifo files
    sprintf(PIPE_ANSWER, "/tmp/parking_answer_%d", client_req.client_pid);

    /* Answer fifo file creation */
    returnValue_answer = mkfifo(PIPE_ANSWER, 0600);
    if (returnValue_answer == -1){
        printf("Answer pipe file already exists, working with the already existing file...\n");
        log_message("[CLIENT][WARNING] Answer FIFO already exists, reusing it");
    } else {
        printf("Answer pipe file created successfully !\n");
        log_message("[CLIENT][INFO] Answer FIFO created successfully");
    }

    /* Open the request fifo file to make a request to the server */ 
    tub_nomme_request = open(PIPE_REQUESTS, O_WRONLY);
    if (tub_nomme_request == -1){
        printf("Error : Unable to open request pipe file, client shutting down...\n");
        log_message("[CLIENT][ERROR] Unable to open request FIFO, shutting down");

        /* Clean up*/
        close(tub_nomme_request);
        unlink(PIPE_ANSWER);

        exit(EXIT_FAILURE);
    }
    log_message("[CLIENT][INFO] Request FIFO opened for writing");

    // Write the request struct pointer to the server 
    write(tub_nomme_request, &client_req, sizeof(request_t));
    log_message("[CLIENT][INFO] Request sent to server");

    /* Answer fifo file opening for reading */
    tub_nomme_answer = open(PIPE_ANSWER, O_RDONLY);
    if (tub_nomme_answer == -1){
        printf("Error : Unable to open answer pipe file, client shutting down...\n");
        log_message("[CLIENT][ERROR] Unable to open answer FIFO, shutting down");
        exit(EXIT_FAILURE);
    }
    log_message("[CLIENT][INFO] Answer FIFO opened for reading");

    // Read the response sent by the server
    read(tub_nomme_answer, &client_res, sizeof(response_t));

    // Log response
    snprintf(log_buf, sizeof(log_buf),
             "[CLIENT][RESPONSE] Received response: car_id=%d status=%d",
             client_res.car_id,
             client_res.status);
    log_message(log_buf);


    // Print the response in human readable format
    printf("Received response from the server :\n");
    print_response(client_res);

    // Clean up
    close(tub_nomme_answer);
    close(tub_nomme_request);

    log_message("[CLIENT][INFO] FIFOs closed");

    // Unlink the answer pipe after getting done with it
    unlink(PIPE_ANSWER);

    log_message("[CLIENT][INFO] Answer FIFO unlinked");

    exit(EXIT_SUCCESS);
}

void client_req_set(request_t *client_req, pid_t PID, int car_id, char action[]){
    client_req->client_pid = PID;
    client_req->car_id = car_id;

    if (!(strcmp(action, "enter"))){ // ENTER action demanded
        client_req->action = ENTER;
    } else if (!(strcmp(action, "exit"))){ // EXIT action demanded
        client_req->action = EXIT;
    } else { // Request doesn't correspond to any known ones (enter or exit)
        printf("Error : Unknown request, try with enter or exit.\n");
        log_message("[CLIENT][ERROR] Unknown action requested");
        exit(EXIT_FAILURE);
    }
}

/* Helper to transform action_t to string */
const char *action_to_string(action_t action){
    switch (action){
        case ENTER:   return "ENTER";
        case EXIT:    return "EXIT";
        default:      return "UNKNOWN";
    }
}