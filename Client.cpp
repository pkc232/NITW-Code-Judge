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

void sendMsg(int sfd, string msg){
	int len = msg.length();
	write(sfd, msg.c_str(), len);
}

int sendFile(int sfd, string fileName){
	ifstream fin;
	fin.open(fileName.c_str());

	string wrd;
	while(!fin.eof()){
		fin>>wrd;
		sendMsg(sfd, wrd);
		sleep(0.5);
	}
	sendMsg(sfd, "ENDIT");
	fin.close();
	cout<<"File Sent\n";
}
int main(int argc, char const *argv[])
{
	string IPServ = "192.168.43.44";
	int port_no = 8080;

	int sfd = establishConnection(IPServ, port_no);
	sendFile(sfd, "wikiSend.txt");

	return 0;
}