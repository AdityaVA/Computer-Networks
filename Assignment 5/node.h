#include <bits/stdc++.h>
using namespace std;
#ifndef NODE_H_
#define NODE_H_
extern int DEBUG;
class Node{
    public:
    int id;
    int sockfd;
    vector<int> neighbours;
    map<int, int> minimum_cost, maximum_cost;
    fstream in;
    fstream out;
    map<int,map<int, int>> adj;
    mutex adj_lock, print_lock;
    Node();
    void node(int id, string infile, string outfile, int HELLO_INTERVAL, int LSA_INTERVAL, int SPF_INTERVAL);
    void hello_send(int HELLO_INTERVAL);
    void LSA_send(int LSA_INTERVAL);
    void receive();
    int random_gen(int mn, int mx);
    void ospf(int SPF_INTERVAL);
};
#endif