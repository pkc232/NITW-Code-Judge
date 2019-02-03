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
7.	Remove all the temporary files created.
8.	Store the result of the corresponding client in a database.
9.	Maintain separate threads for each checking which is being done.

Advanced Functionalities:
1.	Implement a sandbox.
2.	Implement a time counter to give a TLE.


@author: PKC


****/



#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#include "COMMON_FUNCTIONALITY.h"

using namespace std;

int CHILD_PID = -1;
int TIME_LIMIT = 3;

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

string redirect_error_to_file(string command, string error_file_path){
	//This is to redirect the compilation error to the error file.
	command += " 2> ";
	command += error_file_path;
	// command += " 2>&1 ";
	return command;
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

	
	compilation_command			= redirect_error_to_file(compilation_command, error_file_path);
	
	system(compilation_command.c_str());

	if(emptyfile(error_file_path))
		return 1;
	else
		return 0;
}

void kill_child(int sig)
{
	/**
		This method is used to terminate a child process.
	**/
	cout<<"killing "<<CHILD_PID<<endl;
    kill(CHILD_PID,SIGKILL);
}

int executeCode(){
	/**
		This method executes the binary file.
		It redirects the standard input to come from a given input file and 
		send output to a temporary output file.
	**/
	string binary_file_path = FILE_BASE_PATH;
	binary_file_path += "BINARY/cppbin";
	string error_file_path = FILE_BASE_PATH;
	error_file_path += "ERROR_FILES/errorfile.txt";

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

		execv(binary_file_path.c_str(), NULL);
	}
	else
	{
		signal(SIGALRM,(void (*)(int))kill_child);
		CHILD_PID = pid;
		alarm(TIME_LIMIT);
		int status;
		int w = wait(&status);
		if(status == 9){
			//Time Limit Exceeded
			return 2;
		}
		else if(status == 0){
			//Execution Done Completely
			return 1;
		}
		else{
			//Run time error
			return 0;
		}

	}
}

int main(int argc, char const *argv[])
{
	
	int connfd = 3;

	string temp_ip_file = SERVER_FILE_PATH;
	temp_ip_file += "sampleIP.cpp";

	int p = recvFile(connfd, temp_ip_file);
	if(p)
		cout<<"File successfully received\n";

	int cp = compileCode(temp_ip_file);
	string msgToSend = "";
	if(cp == 0){
		msgToSend = "Compilation error\n";
		sendMsg(connfd, msgToSend.c_str());
		return 0;
	}
	
	int exp = executeCode();

	if(exp == 0){
		msgToSend = "Runtime error\n";
		sendMsg(connfd, msgToSend.c_str());
		return 0;
	}

	if(exp == 2){
		msgToSend = "Time Limit Exceeded\n";
		sendMsg(connfd, msgToSend.c_str());
		// cleaup();
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