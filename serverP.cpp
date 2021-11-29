/**
**
** serverP.cpp
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include <stack>

// Used to get connected graph nodes
#include "graph.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

#define IPADDRESS "127.0.0.1"  // local IP address

#define MYPORT "23131"	// the port used for UDP connection with Central
#define MAPDIR "./edgelist.txt" // file directory to get map information
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
void split_args2(char *args[], char *message, string character) 
{
    char *p = strtok (message, character.c_str());
    int i = 0;
    while (p != NULL)
    {
        args[i++] = p;
        p = strtok (NULL, character.c_str());
    }
}

// this function transfer a two line char array to a map
map<string,int> arryToMap(map<string,int> map, char *scores)
{
    char *args[2];
    string name;
    int score;
    string data = scores;
    stringstream ss(data);
    string item;
    // First, use "\n" to split each line
    while (getline(ss, item, '\n')) {
        // then, use " " to split the name and score
        char *p=(char*)item.data();
        split_args2(args, p, " ");
        name = args[0];
        score = atoi(args[1]);
        map.insert(pair<string, int>(name, score));
    }
    return map;
}

string doubleToString(const double &val)
{
    
    char* chCode;
    chCode = new char[20];
    sprintf(chCode, "%.2lf", val);
    std::string str(chCode);
    delete[]chCode;
    return str;
}


///////////////////// functions of using Distra to find the shortest path/////////////////////
Graph<string>  establishGraph(Graph<string> g, char *topo, map<string,int> score)
{
    char *args[2];
    string name1, name2;
    string data = topo;
    stringstream ss(data);
    string item;
    while (getline(ss, item, '\n')) {
        // then, use " " to split the name and score
        char *p=(char*)item.data();
        split_args2(args, p, " ");
        name1 = args[0];
        name2 = args[1];
        // add nodes to graph
        g.add_vertex(name1);
        g.add_vertex(name2);
        // add edges to graph
        double divider = abs(score[name1]-score[name2]);
        double value = divider/(score[name1]+score[name2]);
        g.add_edge(name1, name2, value);
    }
    return g;
}
///////////////////////////////////////////////////////////////////////////////////

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
    
    printf("The ServerP is up and running using UDP on port %s.\n", MYPORT);
    //fflush(stdout);

    while(1) {
        if((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';

        printf("The ServerP received the topology and score infomation.\n");

        // use the topology and score to establish the graph
        // memset(&buf, 0, sizeof buf);
        Graph<string> g;
        split_args2(args, buf, "|");     // split the topology and score
        string score = args[1];
        // create a map structure to store the score
        map<string, int> scoreMap;
        scoreMap = arryToMap(scoreMap,args[1]);

        /////// test the map build ///////
        // map<string, int>::iterator iter;
        // iter = scoreMap.begin();
        // while(iter != scoreMap.end()) {
        //     cout << iter->first << " : " << iter->second << endl;
        //     iter++;
        // }

        // use the topology and score map to build the graph
        g = establishGraph(g, args[0],scoreMap);
        // g.show();

        // get the two names from the third parameter
        char *names[2];
        split_args2(names,args[2]," ");

        // result contains shortest distance and lastnode from source node to all the other nodes
        map<string,shortestPath> result = g.dijkstra(names[0]);
        // cout<<result[names[1]].distance<<endl;
        // cout<<result[names[1]].lastNode<<endl;
        string name1 = names[0];
        string name2 = names[1];
        string lastName = name2;
        string tempString = lastName;
        while(lastName != name1){
            lastName = result[lastName].lastNode;
            tempString = tempString + "|" + lastName;
        }

        string sendBack = tempString + "|" + doubleToString(result[name2].distance) + "|\n";

        // since data that send to central should be char[], we need to transfer string to char[]
        char udp_send[MAXBUFLEN];
        strcpy(udp_send, sendBack.c_str());

        // send the result(data) to Central server
        numbytes = sendto(sockfd, udp_send, strlen(udp_send), 0,
            (struct sockaddr *)&their_addr, addr_len);
        
        if(numbytes == -1) {
            perror("listener: sendto");
            exit(1);
        }

        if(strcmp(data, MAPNOTFOUND) == 0) {
            printf("The ServerP fails to send the results to Central.\n");
        } else {
            printf("The ServerP finished sending the results to the Central.\n");
        }
        
        //close(sockfd);
    }

    return 0;
}