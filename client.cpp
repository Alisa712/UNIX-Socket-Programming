#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

using namespace std;

#define AWSPORT 25298
#define HOST "localhost"
#define BUFFERSIZE 500

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cerr<<"Usage: ./client <function> <input>"<<endl;
		return 1;
	}

	/* Building Socket and Establish TCP Connection */
	char buffer[BUFFERSIZE];

	//create socket package
	struct sockaddr_in AWS_address;
	AWS_address.sin_family = AF_INET;
	AWS_address.sin_port = htons(AWSPORT);
	AWS_address.sin_addr.s_addr = atoi(HOST);

	//creating Socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0); //socket descriptor
	if (sockfd < 0) {
		cerr<<"Failed to create client socket"<<endl;
		return 1;
	}


	//socket connection: connect to AWS
	if (connect(sockfd, (struct sockaddr *)&AWS_address, sizeof(AWS_address)) < 0) {
		cerr<<"Failed to Connect to AWS"<<endl;
		close(sockfd);
		return 1;
	}
	cout<<"The client is up and running."<<endl;

	//send message
	string message = string(argv[1]) + " "+string(argv[2]);
	int msgLen = send(sockfd, message.c_str(), message.length()+1, 0);
	if (msgLen < 0) {
		cerr<<"Failed to send Message to server"<<endl;
		close(sockfd);
		return 1;
	}
	cout<<"The client sent < "<<argv[2]<<" > and "<<argv[1]<<" to AWS"<<endl;

	//receive message from AWS
	if ((msgLen = recv(sockfd, buffer, BUFFERSIZE, 0)) < 0) {
		cout<<"Error in recving the result"<<endl;
		close(sockfd);
		return 1;
	}
	cout<<"According to AWS, "<<argv[1]<<" on < "<<argv[2]<<" >: "<<buffer<<endl;
	close(sockfd);
	return 0;
}
