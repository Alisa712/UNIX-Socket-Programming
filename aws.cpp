#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <signal.h>
#include <iostream>

using namespace std;

#define AWSTCP 25298
#define AWSUDP 24298
#define AUDP 21298
#define BUDP 22298
#define CUDP 23298
#define BUFFERSIZE 500
#define HOST "localhost"
#define DECIMAL 50
#define TERMS 6

//Global Variable
int tcp_fd = -1;
int udp_fd = -1;
int client_num = -1;

//exit and clean
void clean_up(int s) {
	if (s != 0) cout<<"Exit with Ctrl + C"<<endl;
	if (tcp_fd >= 0) close(tcp_fd);
	if (udp_fd >= 0) close(udp_fd);
	exit(1);
}

//helper function to create socket
void sock_assignment(struct sockaddr_in &input, int port) {
	input.sin_family = AF_INET;
	input.sin_port = htons(port);
	input.sin_addr.s_addr = atoi(HOST);
}

//function to communicate with client
int communicate_to_client(char *buffer) {
	memset(buffer, '\0', BUFFERSIZE); //clean buffer
	struct sockaddr_in client_info;
	socklen_t addrlen = sizeof client_info;
	client_num = accept(tcp_fd, (struct sockaddr *) &client_info, &addrlen);
	if (client_num < 0) {
		cout<<"Error in Communicate with client"<<endl;
		clean_up(0);
	}
	int msgLen = recv(client_num, buffer, BUFFERSIZE, 0);
	if (msgLen < 0) {
		cout<<"Error in receiving from client"<<endl;
		clean_up(0);
	}
	return msgLen;
}

//backend communication
int communicate_to_backend(char *buffer, struct sockaddr_in &receipent, string value, string server_name) {
	memset(buffer, '\0', BUFFERSIZE); //clean buffer
	socklen_t addrlen = sizeof receipent;
	value.erase(value.find_last_not_of('0') + 1, string::npos); //erase tailing zeros
	int msgLen = sendto(udp_fd, value.c_str(), value.length(), 0, (struct sockaddr *) &receipent, addrlen);
	if (msgLen < 0) {
		cout<<"Cannot communicate to the Backend Server "<<server_name<<endl;
		clean_up(0);		
	}
	cout<<"The AWS sent < "<<value<<" > to Backend-Server "<<server_name<<endl;
	//receive message
	msgLen = recvfrom(udp_fd, buffer, BUFFERSIZE, 0, (struct sockaddr *) &receipent, &addrlen);
	if (msgLen < 0) {
		cout<<"Failed to receive from Backend Server "<<server_name<<endl;
		clean_up(0);		
	}
	cout<<"The AWS received < "<<buffer<<" > Backend-Server <"<<server_name<<"> using UDP over port < "<<AWSUDP<<" >"<<endl;
	return msgLen;
}
//function to calculate log
string log_cal(float* num_arr) {
	float log_result = 0.0;
	for (int i = 0; i < TERMS; i++) {
		log_result -= num_arr[i]/(float)(i+1);
	}
	char buffer[DECIMAL];
	sprintf(buffer, "%f",log_result); //store result into buffer
	string result = buffer;
	result.erase(result.find_last_not_of('0') + 1, string::npos); //erase tailing zeros
	return result;
}

//function to calculate div
string div_cal(float* num_arr) {
	float div_result = 1.0;
	for (int i = 0; i < TERMS; i++) {
		div_result += num_arr[i];
	}
	char buffer[DECIMAL];
	sprintf(buffer, "%f",div_result); //store result into buffer
	string result = buffer;
	result.erase(result.find_last_not_of('0') + 1, string::npos); //erase tailing zeros
	return result;
}


int main(void) {
	signal(SIGINT, clean_up); //handle Ctrl+C
	struct sockaddr_in AWS_address;
	int resuse = 1;
	//TCP server setup
	tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_fd < 0) {
		cerr<<"Failed to create client socket"<<endl;
		clean_up(0);
	}
	sock_assignment(AWS_address, AWSTCP);

	//helps in reuse of address and port
	if (setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof resuse) < 0) {
		cerr<<"Failed bind to the Corresponding IP"<<endl;
    	clean_up(0);
	}

	//bind server to host
	if (bind(tcp_fd, (struct sockaddr *) &AWS_address, sizeof AWS_address) < 0) {
		cerr<<"Failed bind to the Corresponding IP"<<endl;
    	clean_up(0);
	}

	//start listening
	if (listen(tcp_fd, SOMAXCONN) < 0) {
		cerr<<"Failed listen to the Corresponding IP"<<endl;
    	clean_up(0);
	}

	//UDP setup
	struct sockaddr_in AWS_udp, serverA_addr, serverB_addr, serverC_addr;
	sock_assignment(AWS_udp, AWSUDP);
	sock_assignment(serverA_addr, AUDP);
	sock_assignment(serverB_addr, BUDP);
	sock_assignment(serverC_addr, CUDP);
	// bind connection
	udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (bind(udp_fd, (struct sockaddr *) &AWS_udp, sizeof AWS_udp) < 0) {
		cerr<<"UDP Failed bind to the Corresponding IP"<<endl;
		clean_up(0);		
	}

    cout<<"The AWS is up and running."<<endl;

    while(1) {
    	char buffer[BUFFERSIZE];
    	int msgLen = communicate_to_client(buffer);
    	float num_arr[TERMS];
    	string final_result = "";
    	//clean and generate data to be presented
    	string input  = "", function="";
    	int index  = 0;
    	for (; buffer[index] != ' '; index++) {
    		function += buffer[index];
    	}
    	index++; //skip the space " "
    	for (; index < msgLen; index++) {
    		input += buffer[index];
    	}
    	cout<<"The AWS received input < "<<input<<" > and function="<<function;
		cout<<" from the client using TCP over port "<<AWSTCP<<endl;

		if (function.compare("LOG") ==0 || function.compare("DIV") == 0) {
			num_arr[0] = atof(input.c_str()); // store 'x'
			string input_x = to_string(num_arr[0]);
			msgLen = communicate_to_backend(buffer, serverA_addr, input_x, "A"); //x^2
			num_arr[1] = atof(buffer);
			msgLen = communicate_to_backend(buffer, serverB_addr, input_x, "B"); //x^3
			num_arr[2] = atof(buffer);
			msgLen = communicate_to_backend(buffer, serverC_addr, input_x, "C"); //x^5
			num_arr[4] = atof(buffer);

			input_x = to_string(num_arr[1]); //x^2
			msgLen = communicate_to_backend(buffer, serverA_addr, input_x, "A"); //get x^4
			num_arr[3] = atof(buffer);
			msgLen = communicate_to_backend(buffer, serverB_addr, input_x, "B"); //get x^6
			num_arr[5] = atof(buffer);

			if (function.compare("LOG") == 0) {			
				final_result = log_cal(num_arr);
			} else if (function.compare("DIV") == 0) {
				final_result = div_cal(num_arr);
			}
			cout<<"Values of powers received by AWS: < ";
			for (int i=0; i<TERMS-1; i++) {
				cout<<num_arr[i]<<", ";
			}
			cout<<num_arr[TERMS-1]<<" >"<<endl;
			cout<<"AWS calculated "<<function<<" on < "<<input<<" >: < "<<final_result<<" >"<<endl;			
		} else {
			final_result = "Error: function name must be one of 'LOG' or 'DIV'";
		}

		msgLen = send(client_num, final_result.c_str(), final_result.length()+1, 0);

		if (msgLen < 0) {
			cout<<"Failed to send calculated result to client"<<endl;
			clean_up(0);
		}
		cout<<"The AWS sent < "<<final_result<<" > to client"<<endl;
    }
    return 0;
}
