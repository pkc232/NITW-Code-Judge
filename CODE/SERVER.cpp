/****


This is the SERVER implementation, i.e. the code which 
would be run on the main server system.

Functionalities to be incorporated:
1.	Keep listening to the requests from the clients. 
2.	Accept the client request and establish a connection
3.	Get the cpp file from the client and store it in temporary file
4.	Compile the cpp file and create an executable from it. 
	Change the file descriptors 
5.	Run the executable on the sample test files. (open corresponding fds)
6.	Get the result and send it to the client.
7.	Store the result of the corresponding client in a database.
8.	Maintain separate threads for each checking which is being done.

Advanced Functionalities:
1.	Implement a sandbox.
2.	Implement a time counter to give a TLE.


@author: PKC


****/



#include "DEFINITIONS.h"
#include "CONSTANTS.h"

using namespace std;
#define MAX_PENDING_REQUESTS 10

void set_reuse_addr(int sockfd){
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    	cout<<"setsockopt(SO_REUSEADDR) failed"<<endl;
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

int isSame(string &fileName1, string &fileName2){
	/**
		Checks whether two files are same or not.
	**/
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

int emptyfile(string filepath){
	/**
		Checks whether a file is empty or not
	**/
	ifstream pFile;
	pFile.open(filepath);
	return (pFile.peek() == std::ifstream::traits_type::eof());
}

int compileCode(string cpp_file_source){
	/**
		This method takes a cpp_file as a program 
		and compiles it.
		The compilation error if any is directed to an error file.
		
		Params:
		@cpp_file_source:	string file name of the source cpp file

		Return:
				1 if the file successfully compiles without any error
				0 if there is any compilation error.

	**/

	string binary_file_path = FILE_BASE_PATH;
	binary_file_path += "BINARY/cppbin";

	string compilation_command	 = "g++ ";
	compilation_command 		+= cpp_file_source;
	compilation_command			+= " -o ";
	compilation_command			+= binary_file_path;  

	string error_file_path = FILE_BASE_PATH;
	error_file_path += "ERROR_FILES/errorfile.txt";

	//This is to redirect the compilation error to the error file.
	compilation_command += " > ";
	compilation_command += error_file_path;
	compilation_command += " 2>&1 ";

	// cout<<"Compilation command "<<compilation_command<<endl;
	system(compilation_command.c_str());

	if(emptyfile(error_file_path))
		return 1;
	else
		return 0;
}

int executeCode(){
	/**
		This method executes the binary file.
		It redirects the standard input to come from a given input file and 
		send output to a temporary output file.
	**/
	string binary_file_path = FILE_BASE_PATH;
	binary_file_path += "BINARY/cppbin";

	int pid = fork();

	if(pid == 0){
		cout<<"In child executing the exc\n";
		
		string servin = SERVER_FILE_PATH, servout = SERVER_FILE_PATH;
		servin += "ip.txt";
		close(0);
		int fdr = open(servin.c_str(), O_RDONLY);
		close(1);
		servout += "op.txt";
		int fdw = open(servout.c_str(), O_WRONLY);
		execv(binary_file_path.c_str(),NULL);
	}
	else
	{
		int w = wait(NULL);
		cout<<"Done\n";
	}
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

void check(int connfd){
	// string msg = recvMsg(connfd);
	// cout<<msg<<endl;
	// while(1);
	// sleep(5);
	sendMsg(connfd, "Hello from server");
	close(connfd);
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

	

	string temp_ip_file = SERVER_FILE_PATH;
	temp_ip_file += "sampleIP.cpp";

	int p = recvFile(connfd, temp_ip_file);
	if(p)
		cout<<"File successfully received\n";

	int cp = compileCode(temp_ip_file);
	string msgToSend = "";
	if(cp){
		executeCode();
	}
	else{
		msgToSend = "Compilation error\n";
		sendMsg(connfd, msgToSend.c_str());
		return 0;
	}


	string user_op_file_path = SERVER_FILE_PATH, actual_op_file_path = SERVER_FILE_PATH;
	user_op_file_path 		+= "op.txt";
	actual_op_file_path 	+= "actual_op.txt";

	if(isSame(user_op_file_path, actual_op_file_path))
	{
		msgToSend = "Correct";
		sendMsg(connfd, msgToSend.c_str());
	}
	else
	{
		msgToSend = "Wrong Answer";
		sendMsg(connfd, msgToSend.c_str());
	}
	close(connfd);

	return 0;
}