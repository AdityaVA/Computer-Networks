#include <iostream>
#include <fstream>
#include <string>
#include <set>
using namespace std;
string g_x = "100000111";
int _xor(char * a, string b)
{
    int f=0, ret=0;
    for(int i=0;i<b.length();i++)
    {
        if(f==0 && a[i]=='1')
        {
            ret = i;
            f=1;
        }
        if(a[i] == b[i])
        {
            a[i] = '0';
        }
        else
        {
            a[i] = '1';
        }
    }
    return ret;
}

string remainder(string inp)
{
    string temp = inp;
    string r_x;
    for(int i=0;i<temp.length()-g_x.length()+1;i++)
    {
        if(temp[i] == '1')
        i+=_xor(&temp[i] ,g_x);
    }
    r_x = temp.substr(temp.length()-g_x.length()+1);
    return r_x;
}
string CRC(string inp)
{
    string temp = inp;
    temp += string(g_x.length()-1,'0');
    string r_x = remainder(temp);
    string t_x = inp + r_x;
    return t_x;
}
int main(int argc, char const *argv[])
{
    fstream infile , outfile;
    srand(time(NULL));
    switch(argc)
    {
        case 1:
        case 2:
            cout << "Usage: ./CRC infile outfile" << endl;
            return 0;
        case 3:
            infile.open(argv[1], ios::in);
            if(!infile)
            {
                cout << "Input file not found" << endl;
                return 0;
            }
            outfile.open(argv[2], ios::out);
            break;
        default:
            cout << "Too many arguments" << endl;
            return 0;
    }
    string m_x;
    int c=  0;
    while(infile>>m_x)
    {
        outfile << "Test Case " << ++c << endl;
        outfile << "Original String:          " << m_x << endl;
        string crc_x = CRC(m_x);
        outfile << "Original String with CRC: " << crc_x << endl<<endl;
        outfile << "Random Bit Errors: "<<endl;
        string corrupted_string;
        set<int> errs_introduced;
        while(errs_introduced.size() < 10)
        {
            errs_introduced.insert(2*(rand()%23)+3);
        } 
        for(int err_introduced:errs_introduced)
        {
            set<int> err_pos;
            while(err_pos.size() < err_introduced)
            {
                err_pos.insert(rand()%crc_x.length());
            }
            corrupted_string = crc_x;
            for(auto i:err_pos)
            {
                corrupted_string[i] = (corrupted_string[i]=='0')?'1':'0';
            }
            outfile << "Corrupted String:         " << corrupted_string << endl;
            outfile << "Number of Errors Introduced: " << err_introduced << endl;
            string rem = remainder(corrupted_string);
            outfile << "Remainder: " << rem << endl;
            outfile << "CRC Check: Error Detection " << (rem!="00000000"? "Passed":"Failed") << endl<<endl;
        }
        outfile << "Bursty Errors: "<<endl;
        set<int> start_end ;
        while(start_end.size() < 5)
        {
            start_end.insert(99 + rand()%11);
        }
        for(auto i:start_end)
        {
            corrupted_string = crc_x;
            for(int j=i;j<i+6;j++)
            {
                corrupted_string[j] = (corrupted_string[j]=='0')?'1':'0';
            }
            outfile << "Corrupted String:         " << corrupted_string << endl;
            outfile << "Number of Errors Introduced: " << 6 << endl;
            outfile << "Start: " << i << " End: " << i+6 << endl;
            string rem = remainder(corrupted_string);
            outfile << "Remainder: " << rem << endl;
            outfile << "CRC Check: Error Detection " << (rem!="00000000"?"Passed":"Failed") << endl<<endl;
        }
    }
    // string m_x = "11101000111111101001111001111101011100111101011111100010100110011101110100000101110111011000001111110101001011100100001101101010" ;

    return 0;
}