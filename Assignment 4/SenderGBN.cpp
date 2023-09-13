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
#include <thread>
#include <time.h>
#include <chrono>
#define MAX_ATTEMPTS 5
#define printable(c) ((int)(c)&255)
using namespace std;
int Window_Start = 0;
int MAX_PACKETS = 40; //-n
int DEBUG = 0;  //-d
int WINDOW_SIZE = 10; //-w
int timeout = 100000;
int avg_timeout =0;
int packets_acked = 0;
int packets_sent_all = 0;
int ack_count = 0;
int PACKET_LEN = 512; //-l
int PACKET_GEN_RATE = 10; //-rsocket
chrono::time_point<chrono::high_resolution_clock> program_start;
mutex buffer_mtx, unacked_window_mtx;
condition_variable unacked_notifier;
class Packet
{
    public:
    uint8_t seq;
    string data;
    chrono::time_point<chrono::high_resolution_clock> time_sent;
    int attempts = 0;
};
queue<Packet> unacked_packets;
int seq = 0;
void exit_with_status(int x);
int micro_s(chrono::time_point<chrono::high_resolution_clock> start, chrono::time_point<chrono::high_resolution_clock> end)
{
    return (int)(chrono::duration_cast<chrono::microseconds>(end - start).count());
}
void generate_packets(queue<Packet> &packets, int PACKET_GEN_RATE, int PACKET_SIZE, int MAX_BUFFER_SIZE)
{
    while(ack_count < MAX_PACKETS)
    {   
        for(char i=0; i<PACKET_GEN_RATE; i++)
        {
            if(packets.size() == MAX_BUFFER_SIZE)
                break;           
            Packet p;
            p.seq = seq;
            seq = (seq+1)%256;
            p.data = "";
            for(int j=0; j<PACKET_SIZE; j++)
            {
                p.data += 'a'+rand()%26;
            }
            unique_lock<mutex> lock(buffer_mtx);
            packets.push(p);
        }
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}


void process_ack(int socket_fd)
{
    while(ack_count < MAX_PACKETS)
    {
        char recv_buffer[5];
        sockaddr_in client;
        socklen_t len = sizeof(client);
        int bytes = recvfrom(socket_fd, recv_buffer, sizeof(recv_buffer), 0, (sockaddr*)&client, &len);
       
        if (bytes < 0)
        {
            cout << "Error receiving packet" << endl;
            exit(1);
        }
        uint8_t ack = recv_buffer[4];
        if(DEBUG==1)
            cout << "Received ACK " << printable(ack) << endl;
        if((ack > (Window_Start+WINDOW_SIZE)%256)&&(ack < Window_Start))
        {
            // cerr<<"Received ACK "<< printable(ack) << " out of order"<<endl;
            continue;
        }
        else 
        {
            ack_count++;
            chrono::time_point<chrono::high_resolution_clock> sent_time;
            chrono::time_point<chrono::high_resolution_clock> current_time;
            while(1)
            {
                unique_lock<mutex> lock(unacked_window_mtx);
                if(unacked_packets.front().seq != ack)
                    {unacked_window_mtx.unlock();continue;}
                sent_time = unacked_packets.front().time_sent;
                current_time = chrono::high_resolution_clock::now();
                if(DEBUG == 2)
                {
                    
                    int time_s = micro_s(program_start, current_time); 
                    cout << "Seq #: " << printable(ack) << " Time Generated: " << time_s/1000 << ":" << time_s%1000 
                    << " RTT: " << micro_s(sent_time, current_time) << "us" 
                    << " Number of attempts: "<< unacked_packets.front().attempts << endl;
                }
                Window_Start = ack+1;  
                Window_Start %= 256;
                packets_acked++;
                unacked_packets.pop();
                break;
            }
            current_time = chrono::high_resolution_clock::now();
            avg_timeout = (avg_timeout*(packets_acked-1) + micro_s(sent_time, current_time))/packets_acked;
            if(packets_acked > 10)
                timeout = 2*avg_timeout;
        }
        
    }
}

void send_packets(int s, queue <Packet> &buffer, struct sockaddr_in server)
{   
    while (ack_count < MAX_PACKETS)
    {   
        chrono::time_point<chrono::high_resolution_clock> current_time = chrono::high_resolution_clock::now();
        if(unacked_packets.size()>0)
        {
            if(micro_s(unacked_packets.front().time_sent,current_time) > timeout)
            {
                unique_lock<mutex> lock(unacked_window_mtx);
                for(int i=0;i<unacked_packets.size();i++)
                {
                    if(unacked_packets.front().attempts > MAX_ATTEMPTS)
                    {
                        cout << "Max attempts reached. Exiting" << endl;
                        exit_with_status(1);
                    }
                    Packet packet = unacked_packets.front();
                    unacked_packets.pop();
                    packet.time_sent = chrono::high_resolution_clock::now();
                    packet.attempts++;
                    string data_packet = "a" + packet.data;
                    data_packet[0] = packet.seq;
                    
                    int bytes = sendto(s, data_packet.c_str(), data_packet.length(), 0, (struct sockaddr *)&server, sizeof(server));
                    packets_sent_all++;
                    if (bytes < 0)
                    {
                        cout << "Error sending packet" << endl;
                        exit(1);
                    }
                    if(DEBUG==1)
                        cout << "Sent again packet " << ((int)packet.seq& 255) << endl;
                    unacked_packets.push(packet);
                }
            }
        }
        if(unacked_packets.size() < WINDOW_SIZE)
        {
            unique_lock<mutex> lock(buffer_mtx);
            if(buffer.size() == 0)
                {lock.unlock();continue;}
            Packet packet = buffer.front();
            buffer.pop();
            lock.unlock();
            packet.time_sent = chrono::high_resolution_clock::now();
            packet.attempts++;
            string data_packet = "a" + packet.data;
            data_packet[0] = packet.seq;
            
            int bytes = sendto(s, data_packet.c_str(), data_packet.length(), 0, (struct sockaddr *)&server, sizeof(server));
            packets_sent_all++;
            if (bytes < 0)
            {
                cout << "Error sending packet" << endl;
                exit_with_status(1);
            }
            if(DEBUG==1)
                cout << "Sent packet " << ((int)packet.seq& 255) << endl;
            lock.lock();
            unacked_packets.push(packet);
            lock.unlock();
        }
    }
}


int main(int argc, char* argv [])
{
    // time in microseconds
    program_start = chrono::high_resolution_clock::now();
    char* IP =(char*) malloc(10); //-s
    strcpy(IP, "127.0.0.1");
    int PORT = 8000; //-p
    map<string, char*> ip_map;
    ip_map["localhost"] = new char[10];
    strcpy(ip_map["localhost"], "127.0.0.1");
    ip_map["hemesh"] = new char[10];
    strcpy(ip_map["hemesh"], "10.42.81.48");
    
    int MAX_BUFFER_SIZE = 10; //-f
    for(int i=1;i<argc;i++)
    {
        if(strcmp(argv[i], "-d") == 0)
        {
            DEBUG = 2;
        }
        else if(strcmp(argv[i], "-d1") == 0)
        {
            DEBUG = 1;
        }
        else if(strcmp(argv[i], "-s") == 0)
        {
            IP = argv[i+1];
            cerr<<IP<<endl;
        }
        else if(strcmp(argv[i], "-p") == 0)
        {
            PORT = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-l") == 0)
        {
            PACKET_LEN = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-r") == 0)
        {
            PACKET_GEN_RATE = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            MAX_PACKETS = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-w") == 0)
        {
            WINDOW_SIZE = atoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-f") == 0)
        {
            MAX_BUFFER_SIZE = atoi(argv[i+1]);
        }
    }
    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(IP);
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        cout << "Error creating socket" << endl;
        exit(1);
    }
    queue<Packet> buffer;
    thread gen_thread(generate_packets, ref(buffer), PACKET_GEN_RATE, PACKET_LEN, MAX_BUFFER_SIZE);
    thread ack_thread(process_ack, s);
    thread send_thread(send_packets, s, ref(buffer), server);
    gen_thread.join();
    ack_thread.join();
    send_thread.join();
    exit_with_status(0);
    return 0;
}

void exit_with_status(int x)
{
    cout << endl;
    cout << "STATISTICS:" << endl;
    cout << "PACKET_GEN_RATE: " << PACKET_GEN_RATE << endl;
    cout << "PACKET_LENGTH: " << PACKET_LEN << endl;
    cout << "Retransmission ratio: " << (double)packets_sent_all/packets_acked << endl;
    cout << "Average RTT Value: " << avg_timeout<<" us" << endl;
    exit(x);
}