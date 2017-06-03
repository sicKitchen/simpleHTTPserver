/*
 * A http Client created in C language.
 * Created by Spencer Kitchen
 * For Assignment 2 Part 2 for Networks
 */

/*	TODO:
√	create a connection socket when contacted by a client (browser);
√	receive the HTTP request from this connection;
√	parse the request to determine the specific file being requested;
√	get the requested file from the server’s file system;
√	create an HTTP response message consisting of the requested file preceded by header lines;
√	send the response over the TCP connection to the requesting browser;
√	if a browser requests a file that is not present in your server, your server should return a “404 Not Found” error message;
*/

#include <netinet/in.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/socket.h>    
#include <sys/stat.h>    
#include <sys/types.h>    
#include <unistd.h>    
#include <stdbool.h>
#include <string.h>


//---- DEBUG FLAGS --------------------
#define OUTPUT true			// Set to true to see Server output
#define DEBUG_PARSE false    // Shows DEBUG for parse function
#define DEBUG_MAIN false    // Shows debug for Main function
//-------------------------------------

// For Parser, holds extracted fields
struct Query{
  	char* command;
  	char* path;
};
//-------------- FUNCTIONS -----------------------------------------   
/****** Send a http header with '200 ok' response ******/
// Pass in the destination fp and 
// size of file to create header
void sendHeader(int destination, int size){
	// finds out the length of size in bytes
	int l = snprintf(NULL, 0, "%d", size);
	// allocate memory for length of size plus terminating character
	char* num = malloc(l+1);
	// Copy size(int) into num(string), now we can send the integer 
	// through socket as a string.
	snprintf(num, l+1, "%d", size);

	// Response header
	write(destination, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
	write(destination, "Content-length: ", strlen("Content-length: "));
	write(destination, num, strlen(num));
	write(destination, "\r\n", strlen("\r\n"));
    write(destination, "Content-Type: text/html\r\n", strlen("Content-Type: text/html\r\n"));
    write(destination, "\r\n", strlen("\r\n")); // header end 

    free(num);   
}

/****** Send 'Page not found' if server can not locate file locally ******/
void sendPageNotFound(int destination){
	// message, plus gets length of message and saves it to n
  	char* msg = "<!doctype html><html><head><title>Page Not found</title></head><body><H1>404 ERROR</H1>Page Not Found!!!!!<br></body></html>\n";
  	int n = strlen("<!doctype html><html><head><title>Page Not found</title></head><body><H1>404 ERROR</H1>Page Not Found!!!!!<br></body></html>\n");
	
	// finds out the length of the n in bytes
	int l = snprintf(NULL, 0, "%d", n);
	// allocate memory for length if n plus terminating character
	char* num = malloc(l+1);
	// Copy n(int) into num(string), now we can send the integer 
	// through socket as a string.
	snprintf(num, l+1, "%d", n);

	// RESPONSE HEADER
	write(destination, "HTTP/1.1 404 Not Found\r\n", strlen("HTTP/1.1 404 Not Found\r\n"));
	write(destination, "Content-length: ", strlen("Content-length: "));
	write(destination, num, strlen(num));
	write(destination, "\r\n", strlen("\r\n"));
    write(destination, "Content-Type: text/html\r\n", strlen("Content-Type: text/html\r\n"));
    write(destination, "\r\n", strlen("\r\n")); // header end
    // Body of http request
    write(destination, msg, n);
    free(num); 
}

/**********************************************************************
Parses request string sent form browser filling 2 fields.
	.get 	Holds the GET command sent from browser. For logic control
	.path   Holds the extracted pathway to file requested. 
Returns structure with varaibles filled
***********************************************************************/
struct Query parse_http_header(char* string, const char* delim){
	struct Query result;
	// make copy of string, no mutation
	char* tmp = string;
	// add parsed output to struct
  	result.command = strtok(tmp, delim);	// Get command (ex. GET)
  	char* src = strtok(0, delim); 			// Get pathway
  	if (DEBUG_PARSE) printf("<DEBUG>String from request header: '%s'\n", src);

  	// Copy the pathway without leading '/' to result
  	result.path = malloc(sizeof(src));
  	memcpy(result.path, src+1, strlen(src));

    if(DEBUG_PARSE){ 
    	printf("===== DEBUG =====\n");
    	printf("parse.get: %s\n", result.command);
    	printf("parse.path: %s\n", result.path);
    	printf("=================\n");
	}
	return result;
}

//============= MAIN ===============================================================
int main() {    
  	int create_socket, new_socket, portno;    // for holding sockets and prot numbers
   	portno = 8080;//                <-------------[Change the port number]
   	socklen_t addrlen;    
   	int bufsize = 1024;    
   	char *buffer = malloc(bufsize);    	
   	FILE * file_fp;						// pointer to requested file

   	/********************************************************
   	A sockaddr_in is a struct containing an internet address. 
   	Defined in netinet/in.h.
   	struct sockaddr_in
	{
  		short   sin_family;  // must be AF_INET 
  		u_short sin_port;
  		struct  in_addr sin_addr;
  		char    sin_zero[8];   // Not used, must be zero 
	};
	********************************************************/
   	struct sockaddr_in address;    
 
 	/****** The socket() system call creates a new socket. ******/
   	create_socket = socket(AF_INET, SOCK_STREAM, 0);    
   	if (create_socket < 0){ 
     	perror("ERROR opening socket");
     	exit(1);
   	} else printf("SERVER: Socket opened\n");
    
    // Must do for socket to work
   	address.sin_family = AF_INET;    
   	address.sin_addr.s_addr = INADDR_ANY;    
   	address.sin_port = htons(portno);    				// open on port number provided
    
    /*********************************************************
    The bind() system call binds a socket to an address, 
    the address of the current host and port number 
    **********************************************************/
   	if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) < 0){    
      	perror("ERROR on binding");
      	exit(1);
   	} else printf("SERVER: Waiting for connections...\n\n");
    
   	while (1) {  
   		/****** Listen allows process to listen on the socket for connections. ******/   
      	if (listen(create_socket, 10) < 0) {    
        	perror("server: listen");    
        	exit(1);    
    	}    
    	/****** accept() causes the process to block until client connects. ******/
    	if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) {    
       		perror("server: accept");    
        	exit(1);    
    	}    
    	// we have a connection to server
    	if (new_socket > 0){    
        	printf("SERVER: *** Client connected ***\n");
    	}
    	// print request from browser, server side
    	recv(new_socket, buffer, bufsize, 0);  
    	if (OUTPUT){
    		printf("CLIENT REQUEST:\n");
    	  	printf("> %s\n", buffer);
    	  }  
    	// parse request for file name
    	struct Query request = parse_http_header(buffer, " "); 
    	if(DEBUG_MAIN){
    		printf("===== DEBUG =====\n");
    		printf("Main.command: %s\n",request.command );  
    		printf("Main.command: %s\n",request.path );  
    		printf("=================\n");
      	}

      	if (DEBUG_MAIN){
      		printf("<DEBUG>received request command '%s'\n", request.command );
      	}

      	// If the command was GET, then try to open file
      	if (strcmp(request.command, "GET") == 0){
      		if (OUTPUT) printf("SERVER: Locating File '%s'\n", request.path );
      		file_fp = fopen ( request.path,"r" );
      		
      		// If we have file
      		if (file_fp != NULL){
      			// Send header first
      			struct stat file_length;			// struct to stat system call
      			stat(request.path, &file_length );   // add the stats of requested file tp struct
      			int size = file_length.st_size;     // Get the size of file from struct

      			if (OUTPUT) {
      				 printf("SERVER: File Found! file size = '%d'\n", size );
      				 printf("SERVER: Sending file...\n\n");
      			}
      			sendHeader(new_socket, size);

      			// Send body next
      			while(1){
      				unsigned char buff[256]={0};
      				int numRead = fread(buff,1,256,file_fp);  // read the file 256 bytes at a time
      				if (DEBUG_MAIN) printf("<DEBUG>Bytes read from file: %d \n", numRead);
      			
      				//If read was success, send data. 
            		if(numRead > 0){
                		if (DEBUG_MAIN){
                			printf("\n====== DEBUG ======\n");
                			printf("FILE CONTENTS:\n");
                			printf("%s", buff );
                			printf("\n===================\n");
                		}
                		write(new_socket, buff, numRead);
            		}

            		// If unsuccessful break loop
            		if (numRead < 256){
            			if (DEBUG_MAIN){
            				if (feof(file_fp)) printf("End of file\n\n");
                			if (ferror(file_fp)) printf("Error reading\n\n");
            			}
                		break;
            		}
   				}
   				fclose(file_fp);
   				
      		}else {
      			// Could not open file, send page not found response
      			if (OUTPUT){
      				printf("SERVER: Could not locate file '%s'\n\n", request.path );
      			}
      			sendPageNotFound(new_socket);
      		}
      		// clear buffer for next connection
   			bzero(buffer, bufsize);
      	}
    	close(new_socket);    
  	}    
  	close(create_socket);    
   	return 0;    
}




