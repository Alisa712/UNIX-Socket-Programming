#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <thread>

#define AWSUDP 24298
#define AUDP 21298
#define BUDP 22298
#define CUDP 23298
#define BUFFERSIZE 500
#define HOST "localhost"
#define DECIMAL 50

using namespace std;

//global variable
int A_udp_fd = -1;
int B_udp_fd = -1;
int C_udp_fd = -1;

//exit and clean
void clean_up(int s) {
	if (s != 0) cout<<"Exit with Ctrl + C"<<endl;
	if (A_udp_fd >= 0) close(A_udp_fd);
	if (B_udp_fd >= 0) close(B_udp_fd);
	if (C_udp_fd >= 0) close(C_udp_fd);
	exit(1);
}

//helper function to create socket
void sock_assignment(struct sockaddr_in &input, int port) {
	input.sin_family = AF_INET;
	input.sin_port = htons(port);
	input.sin_addr.s_addr = atoi(HOST);
}

//function to calculate the exponential result
string calculate(string input, float exponent) {
	float base = atof(input.c_str());
	float calculated = pow(base, exponent);
	char buffer[DECIMAL];
	sprintf(buffer, "%g",calculated); //store result into buffer
	
	string result = buffer;
	result.erase(result.find_last_not_of('0') + 1, string::npos); //erase tailing zeros
	return result;
}

//starts three threads for server A, B C
void start_thread(string server) {
	struct sockaddr_in backend_addr, aws_addr;
	int fd = -1;
	int port;
	float exponent;
	string op;
	if (server.compare("A") == 0) {
		port = AUDP;
		fd = A_udp_fd;
		exponent = 2.0;
		op = "square";
	}
	if (server.compare("B") == 0) {
		port = BUDP;
		fd = B_udp_fd;
		exponent = 3.0;
		op = "cube";
	}
	if (server.compare("C") == 0) {
		port = CUDP;
		fd = C_udp_fd;
		exponent = 5.0;
		op = "5th power";
	}

	sock_assignment(backend_addr, port);
	sock_assignment(aws_addr, AWSUDP);

	if (bind(fd, (struct sockaddr *) &backend_addr, sizeof backend_addr) < 0) {
		cerr<<"UDP Failed bind to the Corresponding IP"<<endl;
		clean_up(0);
	}

	cout<<"The Server "<<server<<" is up and running using UDP on port "<<port<<"."<<endl;
	char buffer[BUFFERSIZE];
    //calculate and display results
	while(1) {
		memset(buffer, '\0', BUFFERSIZE); //clean buffer
		socklen_t addrlen = sizeof aws_addr;
		int msgLen = recvfrom(fd, buffer, BUFFERSIZE, 0, (struct sockaddr *) &aws_addr, &addrlen);
		if (msgLen < 0) {
			cout<<"Cannot communicate to the AWS Server"<<endl;
			clean_up(0);
		} 
		string input = buffer;
		cout<<"The Server "<<server<<" received input < "<<input<<" >"<<endl;
		string result = calculate(input, exponent);
		cout<<"The Server "<<server<<" calculated "<<op<<": < "<<result<<" >"<<endl;
		msgLen = sendto(fd, result.c_str(), result.length()+1, 0, (struct sockaddr *) &aws_addr, addrlen);
		if (msgLen < 0) {
			cout<<"Cannot communicate to the AWS Server "<<endl;
			clean_up(0);
		}
		cout<<"The Server "<<server<<" finished sending the output to AWS"<<endl;
	}
}

int main() {
	signal(SIGINT, clean_up); //handle Ctrl+C
	A_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (A_udp_fd < 0) {
		cerr<<"Failed to create backend socket"<<endl;
		clean_up(0);
	}
	B_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (B_udp_fd < 0) {
		cerr<<"Failed to create backend socket"<<endl;
		clean_up(0);
	}
	C_udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (C_udp_fd < 0) {
		cerr<<"Failed to create backend socket"<<endl;
		clean_up(0);
	}

	thread A(start_thread, "A");
	thread B(start_thread, "B");
	thread C(start_thread, "C");

	A.join();
	B.join();
	C.join();

	return 0;
}

