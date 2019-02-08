/****


This is the SERVER implementation, i.e. the code which 
would be run on the main server system.

Functionalities to be incorporated:
1.	Keep listening to the requests from the clients. 
2.	Accept the client request and establish a connection

//The above two functionalities separated out to the super server.

3.	Get the cpp file from the client and store it in temporary file
4.	Compile the cpp file and create an executable from it. 
	Change the file descriptors 
5.	Run the executable on the sample test files. (open corresponding fds)
6.	Get the result and send it to the client.
7.	Remove all the temporary files created.
8.	Store the result of the corresponding client in a database.
9.	Maintain separate threads for each checking which is being done.
// Rather than threads there is a separate process for each checking being done.

Add a timestamp to files to make them unique.

Advanced Functionalities:
1.	Implement a sandbox.
2.	Implement a time counter to give a TLE.


Still To Implement:
1.	A database to store the results.
2.	Sandbox.


@author: PKC


****/



#include "DEFINITIONS.h"
#include "CONSTANTS.h"
#include "COMMON_FUNCTIONALITY.h"

using namespace std;

int CHILD_PID = -1;
int TIME_LIMIT = 3;
string QNO, ROLLNO;


string CPP_SOURCE_FILE_PATH;
string BINARY_FILE_PATH;
string ACTUAL_IP_FILE_PATH;
string ACTUAL_OP_FILE_PATH;
string USER_OP_FILE_PATH;
string ERROR_FILE_PATH;


string generate_temp_file_path(){
	/**
		This method generates a skeleton of the 
		temporary file to be created with the given 
		QNO and ROLLNO.
	**/
	string temp_ip_file = TEMP_FILE_PATH;
	temp_ip_file		+= ROLLNO;
	temp_ip_file		+= "_";
	temp_ip_file		+= QNO;
	return temp_ip_file;
}

void remove_file(string filepath){
	/**
		This method is used to delete a file 
		with the given filepath.
	**/
	int status = remove(filepath.c_str());
	if(status == 0)
	{
		cout<<"File deleted successfully\n";
	}
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
		cout<<"in file 1"<<buf1<<endl;
		cout<<"in file 2"<<buf2<<endl;

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

void create_sandbox(){
	scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_KILL); // default action: kill

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigreturn), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getpid), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(gettid), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(tgkill), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

    // Don't want to give these 3 to child process but execvp requires them
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
    
    seccomp_load(ctx);
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

	string compilation_command	 = "g++ ";
	compilation_command 		+= CPP_SOURCE_FILE_PATH;
	compilation_command			+= " -o ";
	compilation_command			+= BINARY_FILE_PATH;  

	
	compilation_command			= redirect_error_to_file(compilation_command, ERROR_FILE_PATH);
	
	system(compilation_command.c_str());

	if(emptyfile(ERROR_FILE_PATH))
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

	int pid = fork();

	if(pid == 0){
		cout<<"In child executing the exc\n";
		// cout<<"opening "<<ACTUAL_IP_FILE_PATH<<endl;
		close(0);
		int fdr = open(ACTUAL_IP_FILE_PATH.c_str(), O_RDONLY);
		
		close(1);

		int fdw = open(USER_OP_FILE_PATH.c_str(), O_WRONLY);
		create_sandbox();
		// string nbr = ".";
		// nbr += BINARY_FILE_PATH;
		execvp(BINARY_FILE_PATH.c_str(), NULL);
	}
	else
	{
		//This is to check for TLE.
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
			cout<<"THe status is "<<status<<endl;
			return 0;
		}

	}
}

void extract_client_info(string client_info){
	/**
		This method extracts QNO and ROLLNO 
		from the string received from the client.
	**/
	stringstream ss;
	ss<<client_info;
	ss>>ROLLNO>>QNO;
}

int is_correct(){
	return isSame(USER_OP_FILE_PATH, ACTUAL_OP_FILE_PATH);
}

void create_file(string filepath){
	/**
		This method is used to create an empty file 
		with a given filepath.
	**/
	string command 		=	"touch ";
	command 			+=	filepath;
	system(command.c_str());
}

void initialize_file_paths(){
	/**
		This method is used to store the path variables 

	**/
	string temp_file     	=	generate_temp_file_path();
	
	CPP_SOURCE_FILE_PATH	=	temp_file;
	CPP_SOURCE_FILE_PATH	+=	".cpp";

	BINARY_FILE_PATH 		=	temp_file;

	ACTUAL_IP_FILE_PATH		=	INPUT_FILE_PATH;
	ACTUAL_IP_FILE_PATH		+=	QNO;
	ACTUAL_IP_FILE_PATH		+=	".txt";

	ACTUAL_OP_FILE_PATH 	=	OUTPUT_FILE_PATH;
	ACTUAL_OP_FILE_PATH 	+= 	QNO;
	ACTUAL_OP_FILE_PATH 	+= 	".txt";

	USER_OP_FILE_PATH 		=	temp_file;
	USER_OP_FILE_PATH 		+= 	".txt";

	ERROR_FILE_PATH 		= 	temp_file;
	ERROR_FILE_PATH 		+= 	"_error.txt";

	create_file(USER_OP_FILE_PATH);
	create_file(ERROR_FILE_PATH); 		

}

void cleanup(){
	/**
		This method is used to remove the tmporary
		files.
	**/
	return;
	remove_file(USER_OP_FILE_PATH);
	remove_file(ERROR_FILE_PATH);
	remove_file(BINARY_FILE_PATH);
	remove_file(CPP_SOURCE_FILE_PATH);
}

int main(int argc, char const *argv[])
{
	
	int connfd = 3;// This is from the super server.

	

	string client_info = recvMsg(connfd);
	cout<<"Client Info received "<<client_info<<endl;

	extract_client_info(client_info);

	initialize_file_paths();

	string temp_ip_file		= generate_temp_file_path();
	temp_ip_file			+= ".cpp";

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
		close(connfd);
		cleanup();
		return 0;
	}

	if(exp == 2){
		msgToSend = "Time Limit Exceeded\n";
		sendMsg(connfd, msgToSend.c_str());
		close(connfd);
		cleanup();
		return 0;
	}

	

	if(is_correct())
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
	cleanup();

	return 0;
}