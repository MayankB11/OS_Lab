// Group 22 
// Nisarg Shah 15CS10030
// Mayank Bhushan 15CS30019
// Assignment 1b
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

int main()
{
	// Loop to read command line arguments
	while(1)
	{
		// Get the input and tokenize it so as to make it compatible with execvp
		printf("Assignment1b~$ "); 									// for interface
		vector<char*> v;
		int i=-1;
		int count,status,pid,ret;
		string s;

		while(1)													// Store input command in vector of char strings
		{
			cin>>s;
			char* str;
			str=new char[s.size()];
			for(int k=0;k<s.size();k++)
				str[k]=s[k];
			v.push_back(str);
			i++;
			if(cin.peek()=='\n')
				break;
		} 															
		
		count=i+1;
		if(count==1&&s=="quit")
			break;
		char** argc=new char*[count+1]; 							// Store command in char ** argc
		for(i=0;i<count;i++)
		{
			argc[i]=v[i];
		}
		
		pid=fork();												// Fork 
		
		if(pid==0) 
		{
			execvp(argc[0],argc); 									// Exec command in the child process
		}
	    
	    if ((ret = waitpid (pid, &status, 0)) == -1) 				// Wait till the child process is finished(i.e. wait before getting another command)
    	    printf ("Command not found!! \n");
	}
	return 0;
}