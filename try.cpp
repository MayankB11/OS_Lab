#include <sys/types.h>
#include <iostream>
#include <unistd.h>

using namespace std;

int main()
{
    int pid,ppid,cpid;
    cout<<"Main Process with pid : "<<getpid();
    cout<<"\nSplit starts : "<<endl;
    int i=1;
    pid=fork();
    
    if(pid==0)
    {
        cout<<i<<"Child Process with pid :"<<getpid()<<endl;
    }
    else    	
    {	i++;
        cout<<i<<"Parent Process with pid :"<<getpid()<<endl;
    }
}