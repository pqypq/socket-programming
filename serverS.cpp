/**
**
** serverS.cpp
** Author: Qingyuan Peng
** USC-ID#: 8544146131
**
** Code is modified and expanded base on stream server source in beej
** Specificly, backend server implementation refers to listener.c
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

using namespace std;

#define IPADDRESS "127.0.0.1"  // local IP address

#define MYPORT "22131"	// the port used for UDP connection with Central
#define MAPDIR "./scores.txt" // file directory to get map information
#define MAXBUFLEN 4000
#define AWSUDPPORT "24131" // the UDP port of Central

#define MAPNOTFOUND "Map not found"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// split received message
void split_args2(char *args[], char *message) {
    char *p = strtok (message, " ");
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, " ");
    }
}

// read the content of the file to sting and return
string readFileIntoString(const string& path) {
    ifstream input_file(path);
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"
             << path << "'" << endl;
        exit(EXIT_FAILURE);
    }
    return string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
}



int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    char buf[MAXBUFLEN];
    char data[MAXBUFLEN];
    char *args[3];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    // get information of backend server itself
    if((rv = getaddrinfo(IPADDRESS, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    if(p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    addr_len = sizeof their_addr;
    
    printf("The ServerS is up and running using UDP on port %s.\n", MYPORT);
    //fflush(stdout);

    while(1) {
        if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';

        // split messages into name1,name2
        split_args2(args, buf);

        printf("The ServerS received a request from Central to get the scores.\n");

        // read score file and return the content
        // memset(&buf, 0, sizeof buf);

        string res = readFileIntoString(MAPDIR);

        // since data that send to central should be char[], we need to transfer string to char[]
        char udp_send[MAXBUFLEN];
        strcpy(udp_send, res.c_str());

        // send the result(data) to Central server
        numbytes = sendto(sockfd, udp_send, strlen(udp_send), 0,
            (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }

        // if(strcmp(data, MAPNOTFOUND) == 0) {
        //     printf("The ServerT fails to send the topology to Central.\n");
        // } else {
        //     printf("The ServerT finished sending the topology to Central.\n");
        // }

        printf("The ServerS finished sending the scores to Central.\n");
        
        //close(sockfd);
    }

    return 0;
}
