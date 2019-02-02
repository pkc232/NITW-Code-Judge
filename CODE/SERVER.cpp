/****


This is the SERVER implementation, i.e. the code which 
would be run on the main server system.

Functionalities to be incorporated:
1.	Keep listening to the requests from the clients. 
2.	Accept the client request and establish a connection
3.	Get the cpp file from the client and store it in temporary file
4.	Compile the cpp file and create an executable from it. 
	Change the file descriptors 
5.	Run the executable on the sample test files. (open corres fds)
6.	Get the result and send it to the client.
7.	Store the result of the corresponding client in a database.

Advanced Functionalities:
1.	Implement a sandbox.


@author: PKC


****/



#include "DEFINITIONS.h"
#include "CONSTANTS.h"

using namespace std;
#define MAX_PENDING_REQUESTS 10
#define BUFFER_SIZE 1000


int acceptConnection(int listenfd){
	/* 
		Method which accepts connection for the server
		Params: 
		@listenfd:	File descriptor on which the server listens
		
		Return:
		connfd:	The file descriptor on which the connection is established between
				the server and client.The accepted FD.	
				0 if there is some error.	
				
	*/
	int connfd;
	struct sockaddr_in cliaddr;
	socklen_t lencli = sizeof(cliaddr);
	connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&lencli);
	// printf("Connection from %s,port %d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
	
	return connfd;
}

int initializeListener(string IP, int port_no){
	/*
		Method which intializes the socket connection of the server.

		Params:
		@IP: IP on which the server socket is to be established.
		@port_no: The port number on which the server socket is to be established.

		Return:
		listenfd:	The listening fd on which the server listens.
					0 if there is some error.

	*/
	struct sockaddr_in servaddr;
	int listenfd;
	listenfd = socket(AF_INET,SOCK_STREAM,0);

	if(listenfd<0)
	{
		//Socket not created
		return 0;
	}

	//Socket created

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(port_no);

	int bndret = bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

	if(bndret<0)
	{
		cout<<"Bind failed\n";
		return 0;
	}

	//Binding done

	int listret = listen(listenfd,MAX_PENDING_REQUESTS);

	if(listret<0)
	{
		printf("Listen failed\n");
		return 0;
	}

	printf("Socket ready to listen %d\n",listret);

	return listenfd;
}

string recvMsg(int connfd){
	/*
		This method reads the buffer and returns it as a 
		string.

		Params:
		@connfd: The file descriptor on which the server and client communicates.

		Return:

	*/

	char buf[BUFFER_SIZE];

	bzero(buf, sizeof(buf)); // Very important for clearing the buffer. 
							 // Without this the buffer contains random garbage data
	int p = read(connfd,buf,BUFFER_SIZE);
	if(p<=0)
		return "ENDIT";
	string s(buf);
	return s;
}

int recvFile(int connfd, string fileName){
	/**
		This method is used to receive a file on the
		connection file descriptor and store it in the 
		fileName.
		Params:
		@connfd:	The fd on which the file is received
		@fileName:	The temporary file in which the received file is stored.

	**/
	ofstream fout;
	fout.open(fileName.c_str());
	while(1){
		string s = recvMsg(connfd);
		if(s.find("ENDIT") != std::string::npos)
			break;
		fout<<s;
	}
	fout.close();
	return 1;
}

int isSame(string &fileName1, string &fileName2){

	ifstream f1, f2;
	f1.open(fileName1.c_str());
	f2.open(fileName2.c_str());
	char c;
	char buf1[BUFFER_SIZE], buf2[BUFFER_SIZE];
	while(!f1.eof() && !f2.eof()){
		f1.read(buf1, BUFFER_SIZE);
		f2.read(buf2, BUFFER_SIZE);

		if(strcmp(buf1,buf2))
			return 0;		
	}
	return 1;
}

int checkSolution(int connfd){
	string fileName = recvMsg(connfd);
	string newfile = "check";
	newfile += fileName;

	recvFile(connfd, newfile);
	return isSame(fileName, newfile);
}

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

	int connfd = acceptConnection(listenfd);

	string server_file_path = FILE_BASE_PATH;
	server_file_path += "SERVER_FILES/";
	server_file_path += "wikiReceive.txt";

	int p = recvFile(connfd, server_file_path);
	if(p)
		cout<<"File successfully received\n";

	return 0;
}