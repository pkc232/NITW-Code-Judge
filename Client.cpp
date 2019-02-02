/*include all the necessary libraries*/

#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/udp.h>

#include <bits/stdc++.h>

using namespace std;

#define bufferSize 100 

int establishConnection(string IPServ, int port_no){
	int sfd;
	struct sockaddr_in serv_addr;

	bzero(&serv_addr,sizeof(serv_addr));

	if((sfd = socket(AF_INET , SOCK_STREAM , 0))==-1)
	perror("\n socket");
	else printf("\n socket created successfully\n");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_no);
	serv_addr.sin_addr.s_addr = inet_addr(IPServ.c_str());

	if(connect(sfd , (struct sockaddr *)&serv_addr , sizeof(serv_addr))==-1)
	perror("\n connect : ");
	else printf("\nconnect succesful");

	return sfd;
}

void sendMsg(int sfd, char buf[bufferSize]){
	write(sfd, buf, bufferSize);
}

int sendFile(int sfd, string fileName){
	ifstream fin;
	fin.open(fileName.c_str());

	char buf[bufferSize];
	bzero(buf, sizeof(buf));
	
	while(!fin.eof()){
		fin.read(buf, bufferSize);
		// cout<<"read "<<buf<<endl;
		sendMsg(sfd, buf);
		// cout<<"Sent "<<buf<<endl;
		bzero(buf, sizeof(buf));
		sleep(0.1);
	}

	fin.close();
	cout<<"File Sent\n";
}

int checkSolution(int sfd, string fileName){
	sendMsg(sfd, fileName.c_str());
	sendFile(sfd, fileName);
}

int main(int argc, char const *argv[])
{
	string IPServ = "192.168.43.44";
	int port_no = 8080;

	int sfd = establishConnection(IPServ, port_no);
	sendFile(sfd, "op1.txt");

	return 0;
}