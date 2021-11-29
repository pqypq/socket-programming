EE450 Socket Programming Project, Fall 2021

a. Full Name: Qingyuan Peng

b. Student ID: 8544146131

c. What I have done in the assignment:
    I have finished the required phases in Description:
    Phase 1A: Implement two runnable clients, ClientA and ClientB, and a runable Central sercer.
        Establish the TCP connection between clients and Central server so that clients can send names to Central.
    Phase 1B: Implement three runable back-servers(ServerS, ServerT, ServerP).
        Establish the UDP connection between Central and Back-servers
    Phase 2: The Central server will combine two names from clients to string and send to serverT.
            If the two names are not on the same connected graph, the serverT will return an empty string to Central.
        Central will send the "Found no compatibility..." message to clients and no further calculations;
            If the two names are on the same connected graph, the serverT will return the sub graph that names in to Central.
        Then Central will ask the serverS for Scores of the nodes in the graph. After the Central get the graph and the scores, 
        it will combine them into one string and forward it to serverP. Finally, the serverP will match the score to the graph 
        and culculate the matching gap of each edge and use the Dijkstra Algorithm to find the shortest path between two names.
    Phase 3: The serverP puts the shortest route it finds and the matching gap between two names into a string and send back to Central.
        Central will convert the string into a Vector to store the route and the gap value. It will then use the Vector to make up
        and send the route of exect format(<input1> --> <input2>) to clientA and the route(<input2> --> <input1>) to clientB.

d. What my code files are and what each one of them does:
    clientA.cpp:- Code for clientA to communicate with Central Server by TCP;
                - Send the name1 to Central;
                - Request result from Central.
    clientA.cpp:- Code for clientB to communicate with Central Server by TCP;
                - Send the name2 to Central;
                - Request result from Central.
    central.cpp:- Code for Central to communicate with clients by TCP and backend servers by UDP;
                - It get two names from clients and ask the serverT for graph, ask the serverS for scores, then forward them to serverP to culculate the matching gap;
                - At last, forward the result to each client.
    serverT.cpp:- It gets two names from Central using UDP;
                - Read the "edgelist.txt" file;
                - Use depth-first search to find whether the two names are on the same connected graph, and finally return the result to the Central.
    serverS.cpp:- Receive the request from the Central;
                - Read the "scores.txt" file;
                - Send the nodes and scores in connected graph to the Central.
    serverP.cpp:- Receive the topology and scores from the Central;
                - Calculate the matching gap of each edge;
                - Use Dijkstra to find the shortest path;
                - Send the path and the result to Central.
    More code files than the ones mentioned:
        graph.hpp:- Build the Class of "Graph";
                  - Establish the Adjacency list;
                  - Define and relize the function of adding Nodes, adding edges;
                  - Define and relize the function of Depth first search and Dijkstra Algorithm.
        edge.hpp: - Build the Class of "Edge";
                  - Set basic properties of Class "Edge".

e. The format of all the messages exchanged:
    1. clientA: ./clientA Victor
                The client is up and running.
                The client sent Victor to the Central server.
                Found compatibility for Victor and Oliver:
                Victor --- Rachael --- Oliver
                Matching Gap: 1.06
        Data format:
                - The input data will be stored in string array "argv[1]".
    2. clientB: ./clientB Oliver
                The client is up and running.
                The client sent Oliver to the Central server.
                Found compatibility for Oliver and Victor:
                Oliver --- Rachael --- Victor
                Matching Gap: 1.06
        Data format:
                - The input data will be stored in string array "argv[1]".
    3. serverC: ./serverC
                The Central server is up and running.
                The Central server received input="Victor" from the client using TCP over port 25131.
                The Central server received input="Oliver" from the client using TCP over port 26131.
                The Central server sent a request to Backend-Server T.
                The Central server received infomation from Backend-Server T using UDP over port 24131.
                The Central server sent a request to Backend-Server S.
                The Central server received infomation from Backend-Server S using UDP over port 24131.
                The Central server sent a processing request to Backend-Server P.
                The Central server received the results from backend server P.
                The Central server sent the results to client A.
                The Central server sent the results to client B.
        Data format:
                - The data send to ServerT in format of: "name1 name2"
                - The data send to ServerS in format of: "node1 node2'\n'node3 node4'\n'......"
                - The data send to ServerP in format of: "node1 node2'\n'node3 node4'\n'......|name score'\n'name score'\n'......|name1 name2" 
                - The message send back to clients are string array
                - The received data from backend servers are stored in udp_received_serverT[], udp_received_serverS[], and udp_received_serverP[].
    4. serverT: ./serverT
                The ServerT is up and running using UDP on port 21131.
                The ServerT received a request from Central to get the topology.
                The ServerT finished sending the topology to Central.
        Data format:
                - The ServerT sent the message back to Central in format of: "node1 node2'\n'node3 node4'\n'......"
    5. serverS: ./serverS
                The ServerS is up and running using UDP on port 22131.
                The ServerS received a request from Central to get the scores.
                The ServerS finished sending the scores to Central.
        Data format:
                - The ServerS sent the message back to Central in format of: "name1 score1'\n'name2 score2'\n'......"
    6. serverP: ./serverP
                The ServerP is up and running using UDP on port 23131.
                The ServerP received the topology and score infomation.
                The ServerP finished sending the results to the Central.
        Data format:
                - The ServerP sent the message back to Central in format of: "node1|node2|node3|......|matching gap value|"

g. Any idiosyncrasy of my project:
	Under my test case, I have net found any fail. Hoever, to run the project, you must follow the command below:
        1. Frist you should open six different terminal. And using one of them to type"make all" to compile all the file. 
        2. typing "./serverC" using one terminal to run serverC.
        3. typing "./serverT" using one terminal to run serverT.
        4. typing "./serverS" using one terminal to run serverS.
        5. typing "./serverP" using one terminal to run serverP.
        6. typing "./clientA <input1>" using one terminal to run clientA.
        7. typing "./clientB <input2>" using one terminal to run clientB.

h. reuse of the code:
	1. Some code block for setting TCP and UDP are from "Beej's Guide to Network Programming". The variable names are quite similar.
    2. Code of constructing the class of "Graph" and "Edge" refer to the blog: https://blog.csdn.net/weixin_42375679/article/details/112466514. The codes are modified and expanded according to project requirements. They are also marked in the program.
    3. The implementation of Dijkstra Algorithm is based on code from blog: https://blog.csdn.net/weixin_42375679/article/details/113414866. The URL corresponding to github is https://github.com/ShengHangNB/Graph/blob/main/graph.hpp. The code is modified and the function is expanded.
    4. Socket multi-connection method poll() function is learned from: https://pubs.opengroup.org/onlinepubs/009696799/functions/poll.html.
    The code is marked in the program.