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
	if(argc != 4){
		cout<<"Usage ./client <ROLLNO> <QNO> <FILENAME>"<<endl;
		return 0;
	}
	string IPServ = SERVER_IP;
	int port_no = SERVER_PORT;

	int sfd = establishConnection(IPServ, port_no);

	string ROLLNO	=	string(argv[1]);
	string QNO		=	string(argv[2]);
	string FILENAME =	string(argv[3]);

	string client_info	=	ROLLNO;
	client_info			+=	" ";
	client_info			+=	QNO;	

	sendMsg(sfd, client_info.c_str());


	string client_file_path = FILE_BASE_PATH;
	client_file_path += "CLIENT_FILES/";
	client_file_path += FILENAME;
	sendFile(sfd, client_file_path);
	sendMsg(sfd, "ENDIT");


	string msg = recvMsg(sfd);
	cout<<msg<<endl;
	close(sfd);
	return 0;
}