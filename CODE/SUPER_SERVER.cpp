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
			close(3);
			dup(connfd);
			execvp("./server",NULL);
		}
		
	}

	return 0;
}