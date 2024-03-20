#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 3333

#define MIN_ARGS 2
#define MAX_ARGS 2
#define SERVER_ARG_IDX 1

#define USAGE_STRING "usage: %s <server address>\n"

//NOTE: I Only Added Comments on the Functions that I Changed, the Ones with no Comments are Untouched from Lab 7.

void validate_arguments(int argc, char *argv[])
{
    if (argc == 0)
    {
        fprintf(stderr, USAGE_STRING, "client");
        exit(EXIT_FAILURE);
    }
    else if (argc < MIN_ARGS || argc > MAX_ARGS)
    {
        fprintf(stderr, USAGE_STRING, argv[0]);
        exit(EXIT_FAILURE);
    }
}

void send_request(int fd)
{ 
   char *line = NULL;   //Initialize Line Pointer to NULL
   size_t size = 0;     //Initialize Size for getline
   ssize_t num;         //Variable for Number of Characters read by getline
   
   while ((num = getline(&line, &size, stdin)) >= 0){  //Loop Until getline Reaches End of Input
      if (line != NULL){ 			       //Check if line is NULL
         ssize_t bytesSent = write(fd, line, num);     //Variable for the Contents that is being Written
         if (bytesSent < 0){			       //Check if write Failed
            perror("write");
            exit(1);
         } else {				       //Send to Server the Client Request
	    printf("Sent to Server: %s", line);
         }
      }
      char buffer[1024];			       //Buffer to Store Received Data
      ssize_t bytesReceived = read(fd, buffer, 1024);  //Variable to Store read Data
      if (bytesReceived < 0){                          //Check if read Failed
         perror("read");
      } else if (bytesReceived > 0){                   //Print the Received Data from Server
         buffer[bytesReceived] = '\0';                 //Null Terminate the Received Data
         printf("Received from Server: %s\n", buffer);
      }
      free(line);   //Free the Memory Allocated by getline
      line = NULL;  //Set line to NULL
  }  
  free(line);  //Free Outside of the Loop
  close(fd);   //Close fd
}

int connect_to_server(struct hostent *host_entry)
{
   int fd;
   struct sockaddr_in their_addr;

   if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      return -1;
   }

   their_addr.sin_family = AF_INET;
   their_addr.sin_port = htons(PORT);
   their_addr.sin_addr = *((struct in_addr *)host_entry->h_addr);

   if (connect(fd, (struct sockaddr *)&their_addr,
      sizeof(struct sockaddr)) == -1)
   {
      close(fd);
      perror(0);
      return -1;
   }

   return fd;
}

struct hostent *gethost(char *hostname)
{
   struct hostent *he;

   if ((he = gethostbyname(hostname)) == NULL)
   {
      herror(hostname);
   }

   return he;
}

int main(int argc, char *argv[])
{
   validate_arguments(argc, argv);
   struct hostent *host_entry = gethost(argv[SERVER_ARG_IDX]);

   if (host_entry)
   {
      int fd = connect_to_server(host_entry);
      if (fd != -1)
      {
         send_request(fd);
         close(fd);
      }
   }

   return 0;
}

