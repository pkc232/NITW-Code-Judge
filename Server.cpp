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
#define MAX_PENDING_REQUESTS 10

int acceptConnection(int listenfd){

	int connfd;
	struct sockaddr_in cliaddr;
	socklen_t lencli = sizeof(cliaddr);
	connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&lencli);
	printf("Connection from %s,port %d\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
	return connfd;
}

int initializeListener(string IP, int port_no){

	struct sockaddr_in servaddr;
	int listenfd;
	listenfd = socket(AF_INET,SOCK_STREAM,0);

	if(listenfd<0)
	{
		cout<<"Socket not created\n";
		return 0;
	}

	printf("Server created socket %d\n",listenfd);

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(IP.c_str());
	servaddr.sin_port = htons(port_no);

	int bndret = bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

	if(bndret<0)
	{
		cout<<"Bind failed\n";
		return 0;
	}

	printf("Binding done %d\n",bndret);

	int listret = listen(listenfd,MAX_PENDING_REQUESTS);

	if(listret<0)
	{
		printf("Listen failed\n");
	}

	printf("Socket ready to listen %d\n",listret);

	return listenfd;
}

string recvMsg(int connfd){
	char buf[1000];

	bzero(buf, sizeof(buf)); // Very important for clearing the buffer. 
							 // Without this the buffer contains random garbage data
	read(connfd,buf,1000);
	string s(buf);
	return s;
}

int recvFile(int connfd, string fileName){
	ofstream fout;
	fout.open(fileName.c_str());
	while(1){
		string s = recvMsg(connfd);
		if(s.find("ENDIT") != std::string::npos)
			break;
		fout<<s<<" ";
	}
	fout.close();
	return 1;
}

int main(int argc, char const *argv[])
{
	
	string IP = "192.168.43.44";
	int port_no = 8080;

	int listenfd = initializeListener(IP, port_no);
	int connfd = acceptConnection(listenfd);

	int p = recvFile(connfd, "wikiReceive.txt");
	if(p)
		cout<<"File successfully received\n";
	return 0;
}