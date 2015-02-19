#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

		
int MYPORT = 50025;												// Port to run our socket on
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;			// For locking and preventing data races

void *process(void *connfd);

int main(void)
{
	int sfd;									// Holds socket file descriptor
	struct sockaddr_in addr;					// Holds address
	int connfd;									// Connection file descriptor
	int thread_id;								// holds thread iD
	pthread_t thread;							// our thread
	int thread_count = 0;						// Server will stop after 10 threads

	sfd = socket(PF_INET, SOCK_STREAM, 0);		// Initializess socket
	if(sfd == -1)
	{
		printf("ERROR initializing socket.\n");
		return 1;
	}

	memset(addr.sin_zero, 0, sizeof(addr));		// Pads the socket with 0's to make it fit
	addr.sin_family = AF_INET;					// Initialize family to AF_INET
	addr.sin_port = htons(MYPORT);				// Connects the socket we made to our port
	addr.sin_addr.s_addr = INADDR_ANY; 			// Automatically find IP

	// Binds our socket to our port
	int port_test = bind(sfd,(struct sockaddr *)&addr,sizeof(addr)); 
	if(port_test == -1)
	{
		printf("ERROR binding socket to port.\n");
		return 1;
	}	

	// Server will keep going until 10 threads have been made, then it will end.
	// You can remove this condition and just have the server run forever if desired.
	// You can also increase the number of threads above 10 if needed.
	//
	// Wonsun told me that 10 threads was just fine for demonstrating that my server works.
	while(thread_count < 10)		
	{
		// listens for connections to the server
		int server_test = listen(sfd,10);
		if(server_test == -1)
		{
			printf("ERROR setting up port as server port.\n");
			return 1;
		}

		connfd = accept(sfd,NULL,NULL);			// Connection file descriptor	
		thread_id = pthread_create( &thread,NULL,process, &connfd );
		pthread_join(thread,NULL);

		thread_count++;
	}



}

void *process(void *p)
{
	// Convert the void pointer to the connection file descriptor of the connection we accepted
	int *connfd = (int *)p;
	char http_request[1024];								// Holds user's request	
	char file_requested[1000];								// Holds name of file requested by user		
	FILE *user_requested_file;								// Holds the file the user requested
	
	// The reason the server_message needs to be a char pointer is because there are two scenarios that can occur:
	// 1) If a file is found, then we send the reply to the client which includes the date, content length, ect.
	// For this to work, we need to malloc a section of memory to enclose this info.
	// 2) If the file is not found, we send the 404 error message, which is shorter than the reply if a file is found.
	// The 404 error message will also be malloced, but to a different length. 
	// Therefore, depending on if the file is found or not found, the server message will be malloced accordingly.
	char *server_message;

	int content_length;										// Holds the lenght of the user requested file
	int digits_in_content_length;							// Holds the number of digits in our file length
	char *file_content;										// Holds the contents of the user specified file
	time_t curtime;											// Holds the current time
	struct tm *loctime;										// Struct for time
	char string_time[40];									// Holds string version of time	
	int info_sent;											// Holds the return value of the send() function
	int total_info_sent = 0;								// Counter for total information sent by send()
	FILE *output;											// Output stats.txt file

	recv(*connfd,http_request,1024,0); 		// Listen for user request

	if(strncmp(http_request, "GET ", 4) == 0)				// This means we've received an http request
	{
		// We must now check to see if the file the user has requested exists
		//
		// First, we get the name of the file requested
		// The filename we need starts at the 5th character. "GET /" are the first 4 characters, and our file name
		// starts right after that
		//
		int i;
		for (i = 0; i < 1000; i++)
		{
			// We will keep appending characters to our filename until we find a space, a null character, or a new line
			if ((http_request[i+5] == ' ') || (http_request[i+5] == '\n') || (http_request[i+5] == '\0')) 
			{
				break;
			}
			
			file_requested[i] = http_request[i+5]; 
		}

		// Append null at the end of the filename 
		file_requested[i] = '\0';

		// We now attempt to open the file and see if it exists
		user_requested_file = fopen(file_requested, "r");

		if(user_requested_file == NULL)			// If the file is not found, we send error 404
		{
			server_message = (char *)malloc(30 * sizeof(char));
			strcpy(server_message,"\nHTTP/1.1 404 Not Found\n");
		}
		else									// The file has been found, so now we copy send the server reply to client
		{
			// Get content length
			fseek(user_requested_file,0,SEEK_END);
			content_length = ftell(user_requested_file);
			fseek(user_requested_file,0,SEEK_SET);

			// We need to get the number of digits in our file length in order to know
			// how many characters are needed to represent the content length
			int content_length_copy = content_length;
			while(content_length_copy != 0)
			{
				content_length_copy = content_length_copy / 10;
				digits_in_content_length = digits_in_content_length + 1;
			}

			server_message = (char *)malloc( (110*sizeof(char)) + (digits_in_content_length*(sizeof(char))) + (content_length*sizeof(char)));

			// Now we read the file and place it into our buffer
			file_content = (char *)malloc((content_length + 1)*sizeof(char));
			fread(file_content,1,content_length,user_requested_file);
			
			// We must now assemble our server response into our server_message buffer
			strcpy(server_message,"\nHTTP/1.1 200 OK\n");

			// The next piece of info we need for the server response is the current date and time
			curtime = time(NULL);
			loctime = localtime(&curtime);
			strftime(string_time,40,"Date: %A, %d %B %Y %X %Z\n",loctime);

			strcat(server_message,string_time);				// We add it to our server response

			// Next we add the content length
			strcat(server_message,"\nContent-Length: ");
			char string_content_length[digits_in_content_length];
			sprintf(string_content_length,"%d",content_length);	// Converts our content length to a string
			strcat(server_message,string_content_length);

			// Finally, we add the rest of the info, along with the contents of our file at the end
			strcat(server_message,"\nConnection: close\nContent-Type: text/html\n\n");
			strcat(server_message,file_content);

		}
		// Now that we have constructed our server response, we need to send it back to the client
		// There is a chance that our file is larger than our send() command supports, so we must 
		// send the file in pieces if this happens. 
		while(total_info_sent < strlen(server_message))
		{	
			// We add the total_info_sent to our server message as an offeset to pickup where our last packet left off
			// We subtract our total_info_sent from our total server_message in order to have the correct length
			info_sent = send(*connfd,(server_message + total_info_sent),(strlen(server_message) - total_info_sent),0);
			total_info_sent = total_info_sent + info_sent;
		}
		
		// Finally we must write our results to a stats.txt
		// We must synchronize the file to prevent data races because it needs to be accessed exclusively
		pthread_mutex_lock(&mut);				// Lock thread for synchronization
		
		output = fopen("stats.txt","a");		// Open file for appending
		fprintf(output,"\n%s\nClient: IP:PORT",http_request);
		pthread_mutex_unlock(&mut);				// Unlock thread when finished appending to file

	}

} 