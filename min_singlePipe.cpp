#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;
int main()
{

	int temp=0,pid, pip[2];
	char instring[20];

	int maxi = 2147483647;
	int res = pipe(pip); 
	pid = fork();
	if(res==-1){
		perror("Pipe");
		exit(-1);
	}

	if (pid == 0)           /* child : sends message to parent*/
	{
		/* send 7 characters in the string, including end-of-string */
				int arr[5] = {100,24,-100,-1000,-1}; 
		close(pip[0]);
		for(int j=0;j<5;j++){
			cout<<"Parent"<<endl;
			write(pip[1],&arr[j],sizeof(temp));
		}
	}
	else			/* parent : receives message from child */
	{
		/* read from the pipe */
		int j=5;
		close(pip[1]);
		while(j>=0){
			cout<<"Child"<<endl;
			read(pip[0], &temp, sizeof(temp));  
			cout<<temp<<"\t";
			maxi = min(maxi,temp);
			j--;
		}
		cout<<"\nMin : "<<maxi<<endl;
	}
}