/****

This is the super server implementation.
Its functionalities:
1.	Separated out from the server. Now every client first makes a connection with the super server.
2.	The super server forks out a different server process to cater the needs of the client.


@author:	PKC	

****/





#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#include "COMMON_FUNCTIONALITY.h"

using namespace std;


int main(int argc, char const *argv[])
{
	string IP = "192.168.43.44";
	int port_no = 8080;

	int listenfd = initializeListener(IP, port_no);
	if(listenfd == 0)
	{
		cout<<"Unable to initialize connection\n";
		return 0;
	}

	int connfd;

	while(1)
	{
		connfd = acceptConnection(listenfd);

		int pid = fork();
		if(pid == 0){

			//This is done to pass the file descriptor connfd to server.
			close(3);
			dup(connfd);
			execvp("./server",NULL);
		}
		
	}

	return 0;
}