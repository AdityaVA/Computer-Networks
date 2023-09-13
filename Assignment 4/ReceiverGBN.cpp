// NAME: Aditya Vaichalkar
//Roll Number: CS20B084
//Course: CS3205 Jan. 2023 semester
//Lab number: 4
//Date of submission: 5th April 2023
//I confirm that the source file is entirely written by me without
//resorting to any dishonest means.
//Website(s) that I used for basic socket programming code are: None, only previous labs sample code
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>
#define printable(c) ((int)(c)&255)
using namespace std;
chrono::time_point<chrono::high_resolution_clock> program_start;
int pkt_count = 0;
int drop_count = 0;
void exit_with_status(int x);
double RANDOM_DROP_PROB; //-e
bool packetDrop(float RANDOM_DROP_PROB)
{
    int rnd = rand();
    int prob = RAND_MAX * RANDOM_DROP_PROB;
    return rnd <= prob;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    program_start = chrono::high_resolution_clock::now();
    int DEBUG = 0;          //-d
    int PORT;               //-p
    int MAX_PACKETS;        //-n
    
    uint8_t NFE = 0;            // Next Frame Expected
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-d") == 0)
        {
            DEBUG = 2;
        }
        else if(strcmp(argv[i], "-d1")==0)
        {
            DEBUG = 1;
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            PORT = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-n") == 0)
        {
            MAX_PACKETS = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-e") == 0)
        {
            RANDOM_DROP_PROB = atof(argv[i + 1]);
        }
    }
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        cout << "Error binding socket" << std::endl;
        exit(1);
    }
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);
    
    while (pkt_count < MAX_PACKETS)
    {
        char buffer[1024];
        int n = recvfrom(s, buffer, 1024, 0, (struct sockaddr *)&sender, &sender_len);
        chrono::time_point<chrono::high_resolution_clock> time_received = chrono::high_resolution_clock::now();
        int time_received_us = chrono::duration_cast<chrono::microseconds>(time_received - program_start).count();
        if (n < 0)
        {
            cout << "Error receiving packet" << std::endl;
            exit(1);
        }
        if (packetDrop(RANDOM_DROP_PROB))
        {
            if (DEBUG==1)
                cout << "Packet " <<((int)buffer[0] & 255)<< " dropped due to error" << endl;
            if(DEBUG==2)
                // Seq #:   Time Received: xx:yy Packet dropped: true
                cout << "Seq #: " << ((int)buffer[0] & 255) << " Time Received: " << time_received_us/1000 << ":" << time_received_us%1000 << " Packet dropped: true (due to error)" << endl;
            drop_count++;
            continue;
        }
        else
        {
            uint8_t seq = buffer[0];

            if (seq != NFE)
            {
                if (DEBUG==1)
                    cout << "Packet " << ((int)seq & 255)<< " dropped as NFE = " <<((int)NFE & 255)<< endl;
                if(DEBUG==2)
                    // Seq #:   Time Received: xx:yy Packet dropped: true
                    cout << "Seq #: " << ((int)seq & 255) << " Time Received: " << time_received_us/1000 << ":" << time_received_us%1000 << " Packet dropped: true (seq != NFE:" << printable(NFE) << ")" << endl;
                drop_count++;
                continue;
            }
            else
            {
                if (DEBUG==1)
                    cout << "Packet " << ((int)NFE & 255) << " received" << endl;
                if(DEBUG==2)
                    // Seq #:   Time Received: xx:yy Packet dropped: false
                    cout << "Seq #: " << ((int)NFE & 255) << " Time Received: " << time_received_us/1000 << ":" << time_received_us%1000 << " Packet dropped: false" << endl;
                pkt_count++;
            }
        }
        
        char send_buffer[5];
        sprintf(send_buffer, "ACK ");
        send_buffer[4] = NFE;
        int bytes = sendto(s, send_buffer, sizeof(send_buffer), 0, (struct sockaddr *)&sender, sender_len);
        if (bytes < 0)
        {
            cout << "Error sending packet" << std::endl;
            exit(1);
        }
        if(DEBUG==1)
            cout << "ACK " << ((int)NFE & 255) << " sent" << endl;
        NFE++;
        NFE %= 256;
    }
    exit_with_status(0);
}
void exit_with_status(int x)
{
    cout<<endl;
    cout<<"STATISTICS"<<endl;
    cout<<"Total number of packets received: "<< pkt_count << endl;
    cout<<"Packet Drop Probability: "<< RANDOM_DROP_PROB << endl;
    cout<<"Total number of packets dropped: "<< drop_count<< endl;
    exit(x);
}