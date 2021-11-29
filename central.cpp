#define _XOPEN_SOURCE 700
/**
**
** central.cpp
** Author: Qingyuan Peng
** USC-ID#: 8544146131
**
** Code is modified and expanded base on stream server source in beej
** Specificly, the Central Server implementation refers to server.c
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <string>
#include <signal.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <map>
#include <vector>

#include <poll.h>

using namespace std;

#define IPADDRESS "127.0.0.1"  // local IP address

#define CLIENTA_PORT "25131"
#define CLIENTB_PORT "26131"

#define UDP_PORT "24131"

#define SERVERT_PORT "21131"    // the port users will be connecting to Backend-ServerT
#define SERVERS_PORT "22131"    // the port users will be connecting to Backend-ServerS
#define SERVERP_PORT "23131"    // the port users will be connecting to Backend-ServerP

#define MAXBUFLEN 4000
#define BACKLOG 10	 // how many pending connections queue will hold

// from beej
void sigchld_handler(int s){
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    
    while(waitpid(-1, NULL, WNOHANG) > 0);
    
    errno = saved_errno;
}

// funciton to get socket address from a sockaddr struct, from beej
void *get_in_addr(struct sockaddr *sa) {
    if(sa->sa_family == AF_INET6) {
        // IPv6
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }
    // IPv4
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

int getPortNumber(int socket_fd)
{
    struct sockaddr_in sin;
    socklen_t sinlen = sizeof(sin);
    getsockname(socket_fd, (struct sockaddr *)&sin, &sinlen);
    return ntohs(sin.sin_port);
}

////////////////////// Refer from Beej and modified///////////////////////////////
int create_and_bind_tcp_client(const char* port) {
	int rv;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes = 1;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
    }

	for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

	freeaddrinfo(servinfo);

    if(p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

	if(listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

	sa.sa_handler = sigchld_handler; // reap all dead processes sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1); 
    }

    return sockfd;
}

// Just create UDP socket, no send and receive operations
int create_and_bind_udp_client(const char* port)
{
	int sockfd;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
			break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}

	freeaddrinfo(servinfo); // done with servinfo
	return sockfd;
}

// use the same UDP socket to query based on specified port information
void sendUdpQuery(int sockfd, char *query,const char *port, char *data) {
    //int server_sock;
    int numbytes;
    int rv;
    struct addrinfo hints, *servinfo, *p;
	socklen_t addr_len;
	memset(&hints, 0, sizeof hints);
    char recv_data[MAXBUFLEN]; // data received from backend server

	//sockfd = setupUDP(query, port);
    if ((rv = getaddrinfo(IPADDRESS, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

    for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }
        break;
    }

    if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            return;
	}

    //printf("debug: find backend info...\n");
    
    // send map_id, source_vtx and target_vtx to server
	if ((numbytes = sendto(sockfd, query, strlen(query), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	}

    // printf("debug: Central Server has sent query...\n");

	if (strcmp(port, SERVERT_PORT)==0) {
		printf("The Central server sent a request to Backend-Server T.\n");
	} else if (strcmp(port,  SERVERS_PORT)==0) {
		printf("The Central server sent a request to Backend-Server S.\n");
    } else if (strcmp(port,  SERVERP_PORT)==0) {
		printf("The Central server sent a processing request to Backend-Server P.\n");
    } else {
        printf("The Central server sent the request, propagation speed and transmission speed to server C using UDP over port %s.\n", UDP_PORT);
    }

    int recv_bytes;

    recv_bytes = recvfrom(sockfd, recv_data, sizeof recv_data, 0, NULL, NULL);
    if(recv_bytes == -1) {
        perror("recvfrom");
        exit(1);
    }

    recv_data[recv_bytes] = '\0';
    strcpy(data, recv_data);
    //printf("debug: %s\n", data);
    //return return_recv_data;
}

/////////////// Handling longer buffer /////////////////////////
void Send(int fd, char* data, int size) {
    // keep sending until size is zero 
    while (size > 0) {
        int sent_length = write(fd, data, size);
        if (sent_length == 0) {
        fprintf(stderr, "Unexpected error during sending\n");
        close(fd);
        exit(-1);
    }
    // if error not caused by interrrupts
    if (sent_length == -1) {
      if (errno == EINTR)
          continue;
      fprintf(stderr, "Unexpected error during sending\n");
      close(fd);
      exit(-1);
    }
    // updating size and pointers
    size -= sent_length;
    data += sent_length;
  }
}

// split received message
vector<string> subStrToVec(string str,char sep){
    vector<string> vecArr;
    int flagSub = 0;
    for(int i=0;i<str.length();i++){
        if(str[i] == sep){
            string temp = str.substr(flagSub,i-flagSub);
            vecArr.push_back(temp);
            flagSub = i+1;
        }
    }
    return vecArr;
}


int main(){
	// create_and_bind_udp_socket();
	printf("The Central server is up and running.\n");

	int sockfd_A = create_and_bind_tcp_client(CLIENTA_PORT);
	int sockfd_B = create_and_bind_tcp_client(CLIENTB_PORT);
    int udp_sock = create_and_bind_udp_client(UDP_PORT);
	
	int fd_count = 2;

    // poll() function is learn from https://pubs.opengroup.org/onlinepubs/009696799/functions/poll.html
    struct pollfd pfds[fd_count];

    pfds[0].fd = sockfd_A;
    pfds[0].events = POLLIN | POLLOUT; // prepared to read on incoming connection

	pfds[1].fd = sockfd_B;
    pfds[1].events = POLLIN | POLLOUT; // prepared to read on incoming connection

	struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int child_fd[2];
    char ip4[INET_ADDRSTRLEN];		   // space to hold the IPv4 string

    // used for storing the two names from clients, and send them to ServerT
    string args[2];                   // it limits that the clients can only send no more than 2 names to Central
    char udp_received_serverT[MAXBUFLEN];      // buffer to receive map data from backend serverT
    char udp_received_serverS[MAXBUFLEN];      // buffer to receive map data from backend serverS
    char udp_received_serverP[MAXBUFLEN];      // buffer to receive map data from backend serverP

    while(true) {
        int k = 2;
        while (k>0)
        {
            int poll_count = poll(pfds, fd_count, -1);
            if (poll_count == -1)
            {
                perror("Central: poll");
                exit(1);
            }

            for (int i = 0; i < fd_count; i++)
            {
                if (pfds[i].revents & POLLIN)
                {
                    child_fd[i] = accept(pfds[i].fd, (struct sockaddr *)&client_addr, &addr_len);char udp_send[50];
                    if (child_fd[i] == -1)
                    {
                        perror("Central: accept");
                        exit(1);
                    }
                    int numbytes;
                    char buffer[512];
                    if ((numbytes = recv(child_fd[i], buffer, sizeof buffer, 0)) == -1)
                    {
                        perror("Central: recv");
                        exit(1);
                    }
                    buffer[numbytes] = '\0';

                    // print the message on the Central Server's Screen
                    printf("The Central server received input=\"%s\" from the client using TCP over port %d.\n", buffer, getPortNumber(pfds[i].fd));
                    args[i] = buffer;
                }
            }
            k = k-1;
        }

        // merge two names into a  string, send the string in format of "name1 name2" to serverT
        string tempMerge = args[0] + " " + args[1];
        // convert string to char array
        char udp_sendT[50];
        strcpy(udp_sendT, tempMerge.c_str());

        // send the string(two names) to serverT
        sendUdpQuery(udp_sock, udp_sendT, SERVERT_PORT, udp_received_serverT);
        cout<<"The Central server received infomation from Backend-Server T using UDP over port "<<UDP_PORT<<"."<<endl;
        // if the feedback from serverT is empty, then finish the whole task.
        if (udp_received_serverT[0] == '\0'){
            // tell the clients that the results not found and finish the task
            string feedback = "";
            for (int i = 0; i < fd_count; i++){
                if (i == 0) {
                    feedback = "Found no compatibility for " + args[0] + " and " + args[1];
                } else {
                    feedback = "Found no compatibility for " + args[1] + " and " + args[0];
                }
                if (send(child_fd[i], feedback.c_str(), feedback.size(), 0) == -1)
                {
                    perror("Central: send");                                       
                    exit(1);
                }

                close(child_fd[i]);
            }
        } else {
            // The route between name1 and name2 is exist
            // then send request to serverS for asking the scores
            char udp_sendS[50] = "I want scores";
            sendUdpQuery(udp_sock, udp_sendS, SERVERS_PORT, udp_received_serverS);
            cout<<"The Central server received infomation from Backend-Server S using UDP over port "<<UDP_PORT<<"."<<endl;
            // combine the "topology info | scores | name1+name2" into one string
            char udp_sendP[MAXBUFLEN]="";
            strcat(udp_sendP, udp_received_serverT);
            strcat(udp_sendP, "|");
            strcat(udp_sendP, udp_received_serverS);
            strcat(udp_sendP, "|");
            strcat(udp_sendP, udp_sendT);
            // Central forward the topology and scores to serverP
            sendUdpQuery(udp_sock, udp_sendP, SERVERP_PORT, udp_received_serverP);
            cout<<"The Central server received the results from backend server P."<<endl;

            // process the message from P to the format send to clients
            string resultNodes = udp_received_serverP;
            vector<string> vecArr = subStrToVec(resultNodes,'|');

            //////////////// make up the send back message to clientA
            string sendToClientA = "Found compatibility for " + args[0] + " and " + args[1] + ":\n" + args[0];
            for(int i=vecArr.size()-3;i>=0;i--)
            {
                sendToClientA = sendToClientA + " --- " + vecArr[i];
            }
            sendToClientA = sendToClientA + "\nMatching Gap: " + vecArr[vecArr.size()-1] + "\n";
            char feedbackToClientA[MAXBUFLEN];
            strcpy(feedbackToClientA, sendToClientA.c_str());
            //////////////// make up the send back message to clientB
            string sendToClientB = "Found compatibility for " + args[1] + " and " + args[0] + ":\n" + args[1];
            for(int i=1;i<vecArr.size()-1;i++)
            {
                sendToClientB = sendToClientB + " --- " + vecArr[i];
            }
            sendToClientB = sendToClientB + "\nMatching Gap: " + vecArr[vecArr.size()-1] + "\n";
            char feedbackToClientB[MAXBUFLEN];
            strcpy(feedbackToClientB, sendToClientB.c_str());

            // send back the result to clients, and end the clients' tasks
            for (int i = 0; i < fd_count; i++){
                // if (send(child_fd[i], udp_received_serverP, 1000, 0) == -1)
                // {
                //     perror("Central: send");                                       
                //     exit(1);
                // }
                // use for handling longer buffer example
                if(i == 0) {
                    Send(child_fd[i], feedbackToClientA, strlen(feedbackToClientA));
                    cout<<"The Central server sent the results to client A."<<endl;
                } else {
                    Send(child_fd[i], feedbackToClientB, strlen(feedbackToClientB));
                    cout<<"The Central server sent the results to client B."<<endl;
                }

                close(child_fd[i]);
            }
        }
    }

	return 0;
}

