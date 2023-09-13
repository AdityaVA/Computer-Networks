#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
    fstream outfile;
    outfile.open("input.txt", ios::out);
    srand(time(NULL));
    for(int i=0;i<50;i++)
    {
        string inp = "";
        for(int i=0;i<128;i++)
        {
            inp += ('0' + rand()%2);
        }
        outfile << inp << endl;
    }

    outfile.close();
    return 0;
}