/****


This is the CLIENT implementation, i.e. the code which 
would be run on the student's system.

Functionalities to be incorporated:
1.	Establish the connection with the server
2.	Send the cpp program file which needs to be checked.
3.	Wait for the response from the server regarding the result 
	of the submission

@author: PKC


****/


/*include all the necessary libraries*/
#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#include "COMMON_FUNCTIONALITY.h"

using namespace std;


int main(int argc, char const *argv[])
{
	string IPServ = "192.168.43.44";
	int port_no = 8080;

	int sfd = establishConnection(IPServ, port_no);

	string client_file_path = FILE_BASE_PATH;
	client_file_path += "CLIENT_FILES/";
	client_file_path += "sample.cpp";
	cout<<"Sending file\n";
	sendFile(sfd, client_file_path);
	sendMsg(sfd, "ENDIT");


	string msg = recvMsg(sfd);
	cout<<msg<<endl;
	close(sfd);
	return 0;
}