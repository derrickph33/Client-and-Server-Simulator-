#define _GNU_SOURCE
#include "net.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

#define PORT 3333

//NOTE: I Only Added Comments on the Functions that I Changed, the Ones with no Comments are Untouched from Lab 7.

void handle_request(int nfd) {        //This is the same Function from Lab 7, I left it here because I Didn't Know if we Should Keep the echo Functionality or not 
   FILE *network = fdopen(nfd, "r");  
   char *line = NULL;
   size_t size;
   ssize_t num;
   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while((num = getline(&line, &size, network)) >= 0){
        printf("Recieved from Client: %s", line);
        if (num >= 0){
          write(nfd, line, num);
          printf("Sent back to Client: %s", line);
        }
   }
   free(line);
   fclose(network);
}

void handle_request_http(int nfd){      //My Handle Request Function for the GET HTTP Protocol (I Changed run_service to use this Function)
   FILE *network = fdopen(nfd, "r+");   //Open File Stream for Reading
   char *line = NULL;                   //line Pointer Initialized to NULL
   size_t size;                         //size Variable for getline
   ssize_t num;                         //num Variable for getline use
   if (network == NULL) 		//Check if network Failed
   {
      perror("fdopen");
      close(nfd);
      return;
   }

   while((num = getline(&line, &size, network)) >= 0){  //Loop that Handles the Request
        if (line != NULL){                              //Check if line is NULL
           printf("Recieved from Client: %s", line);    //Print the Received Message from Client

           if (strncmp(line, "GET ", 4) == 0){          //Check if Request is GET
	   char fileName[256];                          //Buffer to store fileName
           sscanf(line, "GET %s", fileName);            //Get fileName

           FILE *fp = fopen(fileName, "r");             //Open the File
           if (fp == NULL){                             //Check if Open was Successful
              perror("fopen");
              fprintf(network, "File Not Found\n");
           } else {
              char buffer[1024];                                  //buffer to Store Read Contents of File
              while (fgets(buffer, sizeof(buffer), fp) != NULL){  //Read from the File and Send to Client
                 fprintf(network, "%s", buffer);
              }
              fclose(fp);   //Close fp
           }
         } else {  //Else if there is Invalid Request that isn't GET, send that back
            fprintf(network, "Invalid Request\n");
         }
       }
       free(line);   //Free Memory from getline use
       line = NULL;  //Set line Pointer to NULL
    }
    free(line);      //Free Memory Allocated for line
    fclose(network); //Close the network
}

void child_handler(int sigaction){ 		 //Function that acts as a Signal Handler for SIGCHLD Signals
	while (waitpid(-1, NULL, WNOHANG) > 0);  //Wait for all Terminated Child Processes using waitpid() and WNOHANG
}

void run_service(int fd)
{ 
	struct sigaction action;            //Struct for Signal Handler Using sigaction
	action.sa_handler = child_handler;  //Set the Handler to child_handler
	sigemptyset(&action.sa_mask);       //Initialize Signal Mask to be Empty
	action.sa_flags = SA_RESTART;       //Set the Flags to Automatically Restart Interrupted Calls

	if (sigaction(SIGCHLD, &action, NULL) == -1){ //Register the Signal Handler
		perror("sigaction");
		exit(1);
	}

	while (1)   //Loop which Accepts Connections and Spawns Child Processes
	{ 
		int nfd = accept_connection(fd);  //Accept Connection
		if (nfd != -1)
		{
			printf("Connection established\n"); //Print Statement(s) for Functionality
			pid_t pid = fork();		    //Fork Child Process
			if (pid == -1){   		    //Check if fork Failed
				perror("fork");
				close(nfd);
			} else if (pid == 0) {              //Child Process
				printf("Child Process (PID %d) Handling Client Request\n", getpid()); //for testing
				handle_request_http(nfd);  //Call Handle Request Function which Implemented GET 
				printf("Connection closed\n");
				exit(0);
			} else {   	     //Parent Process
				close(nfd);  //Close nfd
			}
		}
	}
}

int main(void)
{
	int fd = create_service(PORT);

	if (fd == -1)
	{
		perror(0);
		exit(1);
	}

	printf("listening on port: %d\n", PORT);
	run_service(fd);
	close(fd);

	return 0;
}
