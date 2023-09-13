#include <bits/stdc++.h>
#include "node.h"
#include <sys/wait.h>
using namespace std;

void call_node(int id, string infile, string outfile, int HELLO_INTERVAL, int LSA_INTERVAL, int SPF_INTERVAL)
{
    Node n;
    n.node(id, infile, outfile, HELLO_INTERVAL, LSA_INTERVAL, SPF_INTERVAL);
}

int main(int argc, char * argv[])
{
    int id=-1;
    string infile = "input", outfile = "output";
    int HELLO_INTERVAL = 1;
    int LSA_INTERVAL = 5;
    int SPF_INTERVAL = 20;
    int num_nodes = 1;
    for(int i = 1; i<argc ; i+=2)
    {
        if(strcmp(argv[i],"-f")==0)
        {
            infile = argv[i+1];
        }
        else if(strcmp(argv[i], "-n")==0)
        {
            num_nodes = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-o")==0)
        {
            outfile = argv[i+1];
        }
        else if(strcmp(argv[i],"-i")==0)
        {
            id = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-h")==0)
        {
            HELLO_INTERVAL = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-a")==0)
        {
            LSA_INTERVAL = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-s")==0)
        {
            SPF_INTERVAL = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-d")==0)
        {
            DEBUG = 1;
        }
    }
    thread t[num_nodes];
    if(id != -1)
    {
        //call node with id
        call_node(id, infile, outfile, HELLO_INTERVAL, LSA_INTERVAL, SPF_INTERVAL);
    }
    else{
        for(int i=0;i<num_nodes;i++)
        {
            //fork each node
            int f=fork();
            if(f==0)
            {
                //child process
                call_node(i, infile, outfile, HELLO_INTERVAL, LSA_INTERVAL, SPF_INTERVAL);
                break;
            }

        }
    }
    //wait for all nodes to finish
    wait(NULL);

    return 0;    
}