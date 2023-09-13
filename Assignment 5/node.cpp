#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include "node.h"
#define MAXLINE 256
using namespace std;

int DEBUG =0 ;
Node::Node()
{
}
void Node::node(int id, string infile, string outfile, int HELLO_INTERVAL, int LSA_INTERVAL, int SPF_INTERVAL)
{
    this->id = id;
    out.open(outfile+"-"+to_string(id)+".txt", ios::out);
    in = fstream(infile);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    srand(time(0));
    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    int PORT = 10000 + id;
    server.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cout << "Error binding socket" << std::endl;
        exit(1);
    }
    int n, m;
    in >> n >> m;
    neighbours = vector<int>();
    for(int i=0;i<m;i++)
    {
        int u, v, mn, mx;
        in >> u >> v >> mn >> mx;
        if(u == id)
        {
            neighbours.push_back(v);
            minimum_cost[v] = mn;
            maximum_cost[v] = mx;
            // cout<<"Neighbour of "<<id<<" is "<<v<<endl;
        }
        else if(v == id)
        {
            neighbours.push_back(u);
            minimum_cost[u] = mn;
            maximum_cost[u] = mx;
        }
    }

    thread hello_send_thread(&Node::hello_send, this, HELLO_INTERVAL);
    thread LSA_send_thread(&Node::LSA_send, this, LSA_INTERVAL);
    thread receive_thread(&Node::receive, this);
    thread ospf_thread(&Node::ospf, this, SPF_INTERVAL);
    
    hello_send_thread.join();
    LSA_send_thread.join();
    receive_thread.join();
    ospf_thread.join();
}

int Node::random_gen(int mn, int mx)
{
    return mn + rand()%(mx-mn+1);
}

void Node::hello_send(int HELLO_INTERVAL)
{
    string hello_packet = "HELLO " + to_string(id) ;
    while(1)
    {
        struct sockaddr_in nbrnode;
        nbrnode.sin_addr.s_addr = INADDR_ANY;
        nbrnode.sin_family = AF_INET;
        for(auto i: neighbours)
        {
            if(DEBUG)
            {    
                //print_lock.lock();
                out << "Node " <<id << " Sending hello packet to " << i << endl;
                //print_lock.unlock();
            }
            int PORT = 10000+i;
            nbrnode.sin_port = htons(PORT);
            sendto(sockfd, (const char *)hello_packet.c_str(), strlen(hello_packet.c_str()), MSG_CONFIRM, (const struct sockaddr *) &nbrnode, sizeof(nbrnode));
        }
        this_thread::sleep_for(chrono::seconds(HELLO_INTERVAL));
    }
}

void Node::LSA_send(int LSA_INTERVAL)
{
    int seqno = 0;
    while(1)
    {
        string LSA_packet = "LSA ";
        int sz = adj[id].size();
        adj_lock.lock();
        LSA_packet += to_string(id) + " " + to_string(seqno) + " " + to_string(sz) + " ";
        for(auto i: adj[id])
        {
            LSA_packet += to_string(i.first) + " " + to_string(i.second) + " ";
        }
        if(DEBUG)
        {
            //print_lock.lock();
            out <<"Node " <<id << " Sending LSA packet " << LSA_packet << endl;
            //print_lock.unlock();
        }
        adj_lock.unlock();
        struct sockaddr_in nbrnode;
        nbrnode.sin_addr.s_addr = INADDR_ANY;
        nbrnode.sin_family = AF_INET;
        for(auto i: neighbours)
        {
            int PORT = 10000+i;
            nbrnode.sin_port = htons(PORT);
            sendto(sockfd, (const char *)LSA_packet.c_str(), strlen(LSA_packet.c_str()), MSG_CONFIRM, (const struct sockaddr *) &nbrnode, sizeof(nbrnode));
        }
        seqno++;
        this_thread::sleep_for(chrono::seconds(LSA_INTERVAL));
    }
}

void Node::receive()
{
    map<int, int> seq;
    while(1)
    {
        if(DEBUG)
        {
            //print_lock.lock();
            out << "Node " << id << " waiting for packet" << endl;
            //print_lock.unlock();
        }
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        char buffer[MAXLINE];
        memset(buffer, 0, sizeof(buffer));
        recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        if(DEBUG)
        {
            //print_lock.lock();
            out << "Node " << id << " received " << buffer << endl;
            //print_lock.unlock();
        }
        if(buffer[0] == 'H' && buffer[5] == 'R')
        {
            // hello packet
            int nbr, id, cost;
            sscanf(buffer, "HELLOREPLY %d %d %d", &nbr, &id, &cost);
            
            adj_lock.lock();
            adj[id][nbr] = cost;
            adj[nbr][id] = cost;
            adj_lock.unlock();
        }
        else if(buffer[0] == 'H')
        {
            // hello packet
            int nbr;
            sscanf(buffer, "HELLO %d", &nbr);
            if(DEBUG)
            {
                //print_lock.lock();
                out << "Node " << id << " received hello from " << nbr << endl;
                //print_lock.unlock();
            }
            int mn = minimum_cost[nbr];
            int mx = maximum_cost[nbr];
            int cost = random_gen(mn, mx);
            string hello_reply = "HELLOREPLY " + to_string(id) + " " + to_string(nbr) + " " + to_string(cost);  
            struct sockaddr_in nbrnode;
            nbrnode.sin_addr.s_addr = INADDR_ANY;
            nbrnode.sin_family = AF_INET;
            int PORT = 10000+nbr;
            nbrnode.sin_port = htons(PORT);
            sendto(sockfd, (char *)hello_reply.c_str(), strlen(hello_reply.c_str()), MSG_CONFIRM, (const struct sockaddr *) &nbrnode, sizeof(nbrnode));
            if(DEBUG)
            {
                //print_lock.lock();
                out << "Node " << id << " sending hello reply : " << hello_reply << endl;
                //print_lock.unlock();
            }
        }
        else if(buffer[0] == 'L')
        {
            // LSA packet
            stringstream ss(buffer);
            string temp;
            ss >> temp;
            int srcid, seqno ;
            ss >> srcid;
            ss >> seqno;
            int entries;
            ss >> entries;
            if((seq.find(srcid) == seq.end()) 
            ||(seqno > seq[srcid]))
            {
                seq[srcid] = seqno;
                for(int i=0;i<entries;i++)
                {
                    int nbr, cost;
                    ss >> nbr;
                    ss >> cost;
                    adj_lock.lock();
                    adj[srcid][nbr] = cost;
                    adj[nbr][srcid] = cost;
                    adj_lock.unlock();
                }
                struct sockaddr_in nbrnode;
                nbrnode.sin_addr.s_addr = INADDR_ANY;
                nbrnode.sin_family = AF_INET;
                for(auto i: neighbours)
                {
                    if(i==srcid)
                        continue;
                    int PORT = 10000+i;
                    nbrnode.sin_port = htons(PORT);
                    int s = socket(AF_INET, SOCK_DGRAM, 0);
                    sendto(sockfd, (const char *)buffer, MAXLINE, MSG_CONFIRM, (const struct sockaddr *) &nbrnode, sizeof(nbrnode));
                }
            }

            if(DEBUG)
            {
                //print_lock.lock();
                out << "Node " << id << " received LSA packet from " << srcid << endl;
                //print_lock.unlock();
            }

        }

    }
}

void Node::ospf(int ospf_interval)
{
    //run djikstra and print shortest path to each node with the cost
    //run djikstra
    while(1)
    {
        // cout << "Node " << id << " starting calculation" << endl;
        map<int, int> dist;
        map<int, int> parent;
        priority_queue<pair<int, int>> pq;
        adj_lock.lock();
        for(auto i: adj[id])
        {
            dist[i.first] = i.second;
            parent[i.first] = id;
            pq.push({-i.second, i.first});
        }
        adj_lock.unlock();
        dist[id] = 0;
        while(!pq.empty())
        {
            auto it = pq.top();
            int u = it.second;
            pq.pop();
            adj_lock.lock();
            for(auto i: adj[u])
            {
                int v = i.first;
                int w = i.second;
                if(dist.find(v) == dist.end() || dist[v] > dist[u] + w)
                {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({-dist[v], v});
                }
            }
            adj_lock.unlock();
        }
        //print shortest path to each node with the cost
        //print table
        //print_lock.lock();
        out << "----------------------------------------------" << endl;
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);

        out << "Routing Table for node " << id << " at time "<< ctime(&now_c) << endl;
        out << "----------------------------------------------" << endl;
        out << std::setw(13)<< "Destination | Path ";
        out << std::setw(22)<<" | Cost ";
        out << std::setw(5)<<" |" << endl;
        out << "----------------------------------------------" << endl;
        for(auto i:dist)
        {
            // cout << "Node " << id << " printing table" << " with i = " << i.first << endl;
            if(i.first == id)
                continue;
            out<< std::setw(12)<< i.first << "| ";
            int temp = i.first;
            vector<int> path;
            while(temp != id)
            {
                path.push_back(temp);
                temp = parent[temp];
            }
            path.push_back(id);
            reverse(path.begin(), path.end());
            string path_str = "";
            for(int i = 0; i< path.size(); i++ )
            {       

               path_str+= to_string(path[i]);
                if(i != path.size()-1)
                    path_str += " - ";
            }
            out << std::setw(20) << path_str << "|" << std::setw(10) << i.second << "|" << endl;
        }
        //print_lock.unlock();
        // cout << "Node " << id << " finished calculation" << endl;
        this_thread::sleep_for(chrono::seconds(ospf_interval));
    }
}