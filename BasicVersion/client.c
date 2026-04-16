
#include "structures.h"

/* GLOBAL VARIABLES */
int returnValue_answer; // Used to store the return value of the answer fifo creation
int tub_nomme_answer; // used to store the answer fifo file descriptor
int tub_nomme_request; // used to store the request fifo file descriptor 
char PIPE_ANSWER[256]; // used to store the answer pipe fifo file name

/* Prototypes */
void client_req_set(request_t *, pid_t, int, char[]);


int main(int argc, char * argv[]){

    request_t client_req;
    response_t client_res;

    /* Check if the argument count matches the wanted one */
    if (argc < 3){
        printf("Error : Insufficient arguments, need car id and action\n");
        exit(EXIT_FAILURE);
    } else if (argc > 3){
        printf("Error : Too many arguments, enter car id and action only\n");
        exit(EXIT_FAILURE);
    }

    // Client request struct setting to the correct values
    client_req_set(&client_req, getpid(), atoi(argv[1]), argv[2]);

    //add the PID of the client process to distinguish answer fifo files
    sprintf(PIPE_ANSWER, "/tmp/parking_answer_%d", client_req.client_pid);

    /* Answer fifo file creation */
    returnValue_answer = mkfifo(PIPE_ANSWER, 0600);
    if (returnValue_answer == -1){
        printf("Answer pipe file already exists, working with the already existing file...\n");
    } else {
        printf("Answer pipe file created successfully !\n");
    }

    /* Open the request fifo file to make a request to the server */ 
    tub_nomme_request = open(PIPE_REQUESTS, O_WRONLY);
    if (tub_nomme_request == -1){
        printf("Error : Unable to open request pipe file, client shutting down...\n");
        /* Clean up*/
        close(tub_nomme_request);
        unlink(PIPE_ANSWER);

        exit(EXIT_FAILURE);
    }

    // Write the request struct pointer to the server 
    write(tub_nomme_request, &client_req, sizeof(request_t));

    /* Answer fifo file opening for reading */
    tub_nomme_answer = open(PIPE_ANSWER, O_RDONLY);
    if (tub_nomme_answer == -1){
        printf("Error : Unable to open answer pipe file, client shutting down...\n");
        exit(EXIT_FAILURE);
    }

    // Read the response sent by the server
    read(tub_nomme_answer, &client_res, sizeof(response_t));

    // Print the response in human readable format
    printf("Received response from the server :\n");
    print_response(client_res);

    // Clean up
    close(tub_nomme_answer);
    close(tub_nomme_request);

    // Unlink the answer pipe after getting done with it
    unlink(PIPE_ANSWER);

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
        exit(EXIT_FAILURE);
    }
}