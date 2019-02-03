#include "DEFINITIONS.h"
#include "CONSTANTS.h"

#define MAX_PENDING_REQUESTS 10

using namespace std;

void set_reuse_addr(int sockfd){
	/**
		This method is used to make the socket to reuse the port
		if it is already in use.

		Params:
		@sockfd:	The fd of the socket which is already in use.
	**/
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    	cout<<"setsockopt(SO_REUSEADDR) failed"<<endl;
}

int establishConnection(string IPServ, int port_no){
	/**
		
		This method is used to establish a TCP connection between the server and client.
		

		Params:
		@IPServ:	The IP of the server in the form of string.
		@port_no:	An integer indicating the port number on which the service is
					to be provided.
		Return:
		sfd: 		The integer socket file descriptor through which the exchanges 
					will take place.
	**/
	
	int sfd;

	struct sockaddr_in serv_addr;

	bzero(&serv_addr,sizeof(serv_addr));

	if((sfd = socket(AF_INET , SOCK_STREAM , 0)) == -1){
		//Socket failed to create.
		perror("\n socket");
		return -1;
	}

	//Socket created succesfully
	set_reuse_addr(sfd);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_no);
	serv_addr.sin_addr.s_addr = inet_addr(IPServ.c_str());

	if(connect(sfd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) == -1){
		//Connect failed to the server. 
		perror("\n connect : ");
	}

	//Connection successfully established.
	return sfd;
}


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

	set_reuse_addr(listenfd);
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
	cout<<"Done reading the file\n";
	return 1;
}

int sendMsg(int fd, const char buf[BUFFER_SIZE]){
	/**
		This method is used to send a message 
		or a character bytestream on the sfd
		sent as parameter.
		
		Params:
		@fd:	The file descriptor on which the message has 
				to be sent.
		@buf:	The character buffer or message which needs to be sent.

		Include error checking.
	**/

	int er = write(fd, buf, BUFFER_SIZE);

	if(er == -1){
		//Some error occurred in writing the buffer.
		return -1; 
	}

	return 0;
}

int sendFile(int sfd, string fileName){
	/**
		This method is used to send a file.
		add error checking functionality.
	**/

	ifstream fin;
	fin.open(fileName.c_str());

	char buf[BUFFER_SIZE];
	bzero(buf, sizeof(buf));
	
	while(!fin.eof()){
		fin.read(buf, BUFFER_SIZE);
		sendMsg(sfd, buf);
		bzero(buf, sizeof(buf));
		sleep(0.1);
	}

	fin.close();
}

